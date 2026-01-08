/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/events_impl.hpp>
#include <Server/Components/Timers/Impl/timers_impl.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <netcode.hpp>
#include <queue>

using namespace Impl;

class PlayerFixesData;

static bool validateGameText(StringView& message, Milliseconds time, int style)
{
	// ALL styles are recreated here, since even native ones are broken.
	if (time <= Milliseconds::zero())
	{
		// Not shown for any real time.
		return false;
	}
	if (style < 0 || style >= MAX_GAMETEXT_STYLES)
	{
		// Not a style that exists.
		return false;
	}
	// Trim the message.
	size_t len = message.length();
	char const* const data = message.data();
	while (len)
	{
		// rtrim, since TDs don't like trailing spaces.  They do trim the
		// strings themselves, but don't return a failure in that case, so
		// this code won't know about it.
		if (data[len - 1] > ' ')
		{
			break;
		}
		--len;
	}
	if (len == 0)
	{
		// No visible text.
		return false;
	}
	else
	{
		// Constrain the string bounds.
		message = StringView(data, len);
	}
	return true;
}

struct ReapplyAnimationData
{
	PlayerFixesData* data;
	// If the player or actor are destroyed these are set to nullptr.  There's no point worrying
	// about the timer, it isn't a huge strain on resources and will kill itself soon enough.
	// We also remove these pointers if a second animation is applied to this target before the
	// first one has been re-shown, to ensure that we don't get the following case:
	//
	//    1) Unloaded library is applied to actor 1.  Timer is set to reapply.
	//    2) Loaded library is then applied to actor 1.  No timer set.
	//    3) Timer expires and out-of-date animation is re-applied to actor 1.
	//
	// By blanking the pointers when new animations are applied we still ensure that the
	// libraries will be marked as loaded (another excellent reason to not kill the timers) AND
	// ensure that the latest animation is never accidentally wiped out.
	IPlayer* player;
	IActor* actor;
	AnimationData animation;
	ITimer* timer;
};

class PlayerFixesData final : public IPlayerFixesData
{
private:
	IPlayer& player_;
	ITimer* moneyTimer_ = nullptr;
	ITimersComponent& timers_;
	IPlayerTextDrawData* const tds_;
	int money_ = 0;
	StaticArray<IPlayerTextDraw*, MAX_GAMETEXT_STYLES> gts_;
	StaticArray<ITimer*, MAX_GAMETEXT_STYLES> gtTimers_;
	inline static std::deque<ReapplyAnimationData> animationToReapply_ {};

	void GameTextTimer(int style)
	{
		// Hide and destroy the TD.
		if (gts_[style])
		{
			tds_->release(gts_[style]->getID());
			gts_[style] = nullptr;
		}
		gtTimers_[style] = nullptr;
	}

	// TODO: There are so many ways to make this code smaller and faster.  Thus I've just abstracted
	// recording which animation libraries are loaded to these two functions.  Feel free to replace
	// them with a bit map, or string hash, or anything else.  I used a hash anyway, basically free.
	FlatHashSet<size_t> libraries_;

	void See(StringView const lib)
	{
		size_t hash = std::hash<StringView> {}(lib);
		libraries_.insert(hash);
	}

	bool Saw(StringView const lib) const
	{
		size_t hash = std::hash<StringView> {}(lib);
		return libraries_.find(hash) != libraries_.end();
	}

	static void AnimationTimer()
	{
		// Pop the next animation off the queue.
		ReapplyAnimationData const&
			next
			= animationToReapply_.front();
		// Only timer for a disconnected player.
		if (next.data)
		{
			if (next.player)
			{
				// This could be ourselves, or another player, doesn't matter.  There's no need to
				// check the stream type because if this needs fixing it was sent to this player
				// earlier by whatever the sync type was back then.
				NetCode::RPC::ApplyPlayerAnimation RPC(next.animation);
				RPC.PlayerID = next.player->getID();
				PacketHelper::send(RPC, next.data->player_);
			}
			else if (next.actor)
			{
				NetCode::RPC::ApplyActorAnimationForPlayer RPC(next.animation);
				RPC.ActorID = next.actor->getID();
				PacketHelper::send(RPC, next.data->player_);
			}
			/*else
			{
				// They can both be false if the target left during the delay timer.
			}*/
			// Always mark the library as now loaded, because it was shown at least one in the past,
			// even if we didn't need to re-show it now.
			next.data->See(next.animation.lib);
		}
		animationToReapply_.pop_front();
	}

	void MoneyTimer()
	{
		player_.setMoney(money_);
	}

	friend class FixesComponent;

public:
	void freeExtension() override
	{
		delete this;
	}

	PlayerFixesData(IPlayer& player, ITimersComponent& timers)
		: player_(player)
		, timers_(timers)
		, tds_(queryExtension<IPlayerTextDrawData>(player))
	{
		gts_.fill(nullptr);
		gtTimers_.fill(nullptr);
	}

	void startMoneyTimer()
	{
		if (moneyTimer_)
		{
			moneyTimer_->kill();
		}
		// TODO: This must be fixed on client side
		// 50 gives very good results in terms of not flickering.  100 gives OK results.  80 is
		// between them to try and balance effect and bandwidth.
		money_ = player_.getMoney();
		moneyTimer_ = timers_.create(new SimpleTimerHandler(std::bind(&PlayerFixesData::MoneyTimer, this)), Milliseconds(80), true);
	}

	void stopMoneyTimer()
	{
		if (moneyTimer_)
		{
			moneyTimer_->kill();
			player_.setMoney(player_.getMoney());
			moneyTimer_ = nullptr;
		}
	}

	void doHideGameText(int style)
	{
		// Hide and destroy the TD.
		if (gts_[style])
		{
			tds_->release(gts_[style]->getID());
			gts_[style] = nullptr;
		}
		if (gtTimers_[style])
		{
			gtTimers_[style]->kill();
			gtTimers_[style] = nullptr;
		}
	}

	bool doSendGameText(StringView message, Milliseconds time, int style)
	{
		if (!tds_)
		{
			// Don't have a reference to the player's TD component.
			return false;
		}
		// The parameters are validated before this function is called to avoid
		// repetition when showing to all players.
		Vector2 pos;
		// First get the position because we need that to determine if the
		// creation was successful.
		switch (style)
		{
		case 0:
			// Gametext style 0.
			pos = { 320.0, 214.0 };
			break;
		case 1:
			// Gametext style 1.
			pos = { 620.0, 310.0 };
			break;
		case 2:
			// Gametext style 2.
			pos = { 320.0, 156.0 };
			break;
		case 3:
			// Gametext style 3.
			pos = { 320.0, 154.5 };
			break;
		case 4:
			// Gametext style 4.
			pos = { 320.0, 115.5 };
			break;
		case 5:
			// Gametext style 5.
			pos = { 320.0, 217.0 };
			break;
		case 6:
			// Gametext style 6.
			pos = { 320.0, 60.0 };
			break;
		case 7:
			// Gametext style 7 (vehicle name).
			pos = { 608.0, 344.0 };
			break;
		case 8:
			// Gametext style 8 (location name).
			pos = { 608.0, 385.8 };
			break;
		case 9:
			// Gametext style 9 (radio name).
			pos = { 320.0, 22.0 };
			break;
		case 10:
			// Gametext style 10 (radio switch).
			pos = { 320.0, 22.0 };
			break;
		case 11:
			// Gametext style 11 (positive money).
			pos = { 608.0, 77.0 };
			break;
		case 12:
			// Gametext style 12 (negative money).
			pos = { 608.0, 77.0 };
			break;
		case 13:
			// Gametext style 13 (stunt).
			pos = { 380.0, 341.15 };
			break;
		case 14:
			// Gametext style 14 (clock).
			pos = { 608.0, 22.0 };
			break;
		case 15:
			// Gametext style 15 (popup).
			pos = { 34.0, 28.0 };
			break;
		default:
			// Not a valid style, return a failure.
			return false;
		}
		// If this style is already shown hide it first (also handily frees up a
		// TD we might need to create this new GT).  Also stop the timer.
		doHideGameText(style);
		IPlayerTextDraw* td = tds_->create(pos, message);
		if (td == nullptr)
		{
			gts_[style] = nullptr;
			gtTimers_[style] = nullptr;
			return false;
		}
		// And do the rest of the style.
		switch (style)
		{
		case 0:
			// Gametext style 0.
			td->setLetterSize({ 1.3, 3.6 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0x90, 0x62, 0x10, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 1:
			// Gametext style 1.
			td->setLetterSize({ 1.0, 2.6 });
			td->setAlignment(TextDrawAlignment_Right);
			td->setColour(Colour(0x90, 0x62, 0x10, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 10.0, 200.0 });
			break;
		case 2:
			// Gametext style 2.
			td->setLetterSize({ 2.1, 4.2 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0xE1, 0xE1, 0xE1, 0xFF));
			td->setShadow(0);
			td->setOutline(3);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_0);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 3:
			// Gametext style 3.
			td->setLetterSize({ 0.6, 2.75 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0x90, 0x62, 0x10, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 4:
			// Gametext style 4.
			td->setLetterSize({ 0.6, 2.75 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0x90, 0x62, 0x10, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 5:
			// Gametext style 5.
			td->setLetterSize({ 0.6, 2.75 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0xE1, 0xE1, 0xE1, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 6:
			// Gametext style 6.
			td->setLetterSize({ 1.0, 3.6 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0xAC, 0xCB, 0xF1, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 7:
			// Gametext style 7 (vehicle name).
			td->setLetterSize({ 1.0, 3.0 });
			td->setAlignment(TextDrawAlignment_Right);
			td->setColour(Colour(0x36, 0x68, 0x2C, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 10.0, 200.0 });
			break;
		case 8:
			// Gametext style 8 (location name).
			td->setLetterSize({ 1.2, 3.8 });
			td->setAlignment(TextDrawAlignment_Right);
			td->setColour(Colour(0xAC, 0xCB, 0xF1, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_0);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 10.0, 200.0 });
			break;
		case 9:
			// Gametext style 9 (radio name).
			td->setLetterSize({ 0.6, 1.8 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0x90, 0x62, 0x10, 0xFF));
			td->setShadow(0);
			td->setOutline(1);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 10:
			// Gametext style 10 (radio switch).
			td->setLetterSize({ 0.6, 1.8 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0x96, 0x96, 0x96, 0xFF));
			td->setShadow(0);
			td->setOutline(1);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_2);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 200.0, 620.0 });
			break;
		case 11:
			// Gametext style 11 (positive money).
			td->setLetterSize({ 0.55, 2.2 });
			td->setAlignment(TextDrawAlignment_Right);
			td->setColour(Colour(0x36, 0x68, 0x2C, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(false);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 10.0, 200.0 });
			break;
		case 12:
			// Gametext style 12 (negative money).
			td->setLetterSize({ 0.55, 2.2 });
			td->setAlignment(TextDrawAlignment_Right);
			td->setColour(Colour(0xB4, 0x19, 0x1D, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(false);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 10.0, 200.0 });
			break;
		case 13:
			// Gametext style 13 (stunt).
			td->setLetterSize({ 0.58, 2.42 });
			td->setAlignment(TextDrawAlignment_Center);
			td->setColour(Colour(0xDD, 0xDD, 0xDB, 0xFF));
			td->setShadow(2);
			td->setOutline(0);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_1);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 40.0, 460.0 });
			break;
		case 14:
			// Gametext style 14 (clock).
			td->setLetterSize({ 0.55, 2.2 });
			td->setAlignment(TextDrawAlignment_Right);
			// There's some debate over this colour.  It seems some versions
			// somehow end up with `0xE1E1E1FF` instead.
			td->setColour(Colour(0xE1, 0xE1, 0xE1, 0xFF));
			td->setShadow(0);
			td->setOutline(2);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_3);
			td->setProportional(false);
			td->useBox(false);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x00));
			td->setTextSize({ 400.0, 20.0 });
			break;
		case 15:
			// Gametext style 15 (popup).
			td->setLetterSize({ 0.52, 2.2 });
			td->setAlignment(TextDrawAlignment_Left);
			td->setColour(Colour(0xFF, 0xFF, 0xFF, 0x96));
			td->setShadow(0);
			td->setOutline(0);
			td->setBackgroundColour(Colour(0x00, 0x00, 0x00, 0xFF));
			td->setStyle(TextDrawStyle_1);
			td->setProportional(true);
			td->useBox(true);
			td->setBoxColour(Colour(0x00, 0x00, 0x00, 0x80));
			td->setTextSize({ 230.5, 200.0 });
			break;
		}
		// Show the TD to the player and start a timer to hide it again.
		td->show();
		gtTimers_[style] = timers_.create(new SimpleTimerHandler(std::bind(&PlayerFixesData::doHideGameText, this, style)), time, false);
		gts_[style] = td;

		return true;
	}

	bool sendGameText(StringView message, Milliseconds time, int style) override
	{
		// Check the parameters.
		if (!validateGameText(message, time, style))
		{
			return false;
		}
		return doSendGameText(message, time, style);
	}

	bool hideGameText(int style) override
	{
		if (gts_[style])
		{
			doHideGameText(style);
			return true;
		}
		return false;
	}

	bool hasGameText(int style) override
	{
		return !!gts_[style];
	}

	bool getGameText(int style, StringView& message, Milliseconds& time, Milliseconds& remaining) override
	{
		if (gts_[style] && gtTimers_[style])
		{
			message = gts_[style]->getText();
			time = gtTimers_[style]->interval();
			remaining = gtTimers_[style]->remaining();
			return true;
		}
		return false;
	}

	void reset() override
	{
		if (moneyTimer_)
		{
			moneyTimer_->kill();
			moneyTimer_ = nullptr;
		}
		// Hide all gametexts.
		for (int style = 0; style != MAX_GAMETEXT_STYLES; ++style)
		{
			if (gtTimers_[style])
			{
				gtTimers_[style]->kill();
				gtTimers_[style] = nullptr;
			}
			// Don't destroy the TD, the TD component does that.  Just reset the pointer.
			gts_[style] = nullptr;
		}
		// Kill all animation timers for this player.
		for (auto& anim : animationToReapply_)
		{
			if (anim.data == this)
			{
				anim.data = nullptr;
			}
			if (anim.player == &player_)
			{
				anim.player = nullptr;
			}
		}
	}

	void applyAnimation(IPlayer* player, IActor* actor, AnimationData const* animation) override
	{
		// Create a new reapplication.
		if (!Saw(animation->lib))
		{
			PlayerFixesData::animationToReapply_.push_back({
				this,
				player,
				actor,
				*animation,
				timers_.create(new SimpleTimerHandler(&PlayerFixesData::AnimationTimer), Milliseconds(500), false),
			});
		}
	}

	~PlayerFixesData()
	{
		reset();
	}
};

class FixesComponent final : public IFixesComponent, public PlayerConnectEventHandler, public PlayerSpawnEventHandler, public PlayerDamageEventHandler, public ClassEventHandler
{
private:
	IClassesComponent* classes_ = nullptr;
	IPlayerPool* players_ = nullptr;
	ITimersComponent* timers_ = nullptr;
	Microseconds resetMoney_ = Microseconds(0);

public:
	StringView componentName() const override
	{
		return "Fixes";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	FixesComponent()
	{
	}

	~FixesComponent()
	{
		if (players_)
		{
			players_->getPlayerConnectDispatcher().removeEventHandler(this);
			players_->getPlayerSpawnDispatcher().removeEventHandler(this);
			players_->getPlayerDamageDispatcher().removeEventHandler(this);
		}
		if (classes_)
		{
			classes_->getEventDispatcher().removeEventHandler(this);
		}
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
	}

	void onLoad(ICore* c) override
	{
		constexpr event_order_t EventPriority_Fixes = 100;
		players_ = &c->getPlayers();
		players_->getPlayerConnectDispatcher().addEventHandler(this, EventPriority_Fixes);
		players_->getPlayerSpawnDispatcher().addEventHandler(this, EventPriority_Fixes);
		players_->getPlayerDamageDispatcher().addEventHandler(this, EventPriority_Fixes);
	}

	void onInit(IComponentList* components) override
	{
		constexpr event_order_t EventPriority_Fixes = -100;
		classes_ = components->queryComponent<IClassesComponent>();
		if (classes_)
		{
			classes_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
		}
		timers_ = components->queryComponent<ITimersComponent>();
	}

	void onPlayerSpawn(IPlayer& player) override
	{
		// TODO: This must be fixed on client side
		// *
		// * <problem>
		// *     San Andreas deducts $100 from players.
		// * </problem>
		// * <solution>
		// *     Reset the player's money to what it was before they died.
		// * </solution>
		// * <see>OnPlayerSpawn</see>
		// * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		// *
		// This is a minimal implementation on its own as the death money isn't synced properly.
		// However this code will cause the money to flicker slightly while it goes down and up a
		// little bit due to lag.  So instead we pre-empt it with a timer constantly resetting the
		// cash until they spawn.
		PlayerFixesData* data = queryExtension<PlayerFixesData>(player);
		if (data)
		{
			data->stopMoneyTimer();
		}
	}

	bool onPlayerRequestClass(IPlayer& player, unsigned int classId) override
	{
		// TODO: This must be fixed on client side
		// *
		// * <problem>
		// *     Random blunts and bottles sometimes appear in class selection.
		// * </problem>
		// * <solution>
		// *     Call "RemoveBuildingForPlayer".
		// * </solution>
		// * <see>OnPlayerRequestClass</see>
		// * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		// *
		auto pos = player.getPosition();
		player.removeDefaultObjects(1484, pos, 10.0f);
		player.removeDefaultObjects(1485, pos, 10.0f);
		player.removeDefaultObjects(1486, pos, 10.0f);
		return true;
	}

	void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
	{
		PlayerFixesData* data = queryExtension<PlayerFixesData>(player);
		if (data)
		{
			data->startMoneyTimer();
		}

		// TODO: This must be fixed on client side
		// *
		// * <problem>
		// *     Clients get stuck when they die with an animation applied.
		// * </problem>
		// * <solution>
		// *     Clear their animations.
		// * </solution>
		// * <see>OnPlayerDeath</see>
		// * <see>OnPlayerUpdate</see>
		// * <author    >h02</author>
		// * <post      href="https://sampforum.blast.hk/showthread.php?tid=312862&amp;pid=1641144#pid1641144" />
		// *
		auto anim = player.getAnimationData();
		if (anim.ID != 0 && anim.name().first.compare("PED"))
		{
			// Not in a "PED" library so may get stuck.
			NetCode::RPC::ClearPlayerTasks clearPlayerTasksRPC;
			clearPlayerTasksRPC.PlayerID = player.getID();
			PacketHelper::send(clearPlayerTasksRPC, player);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		if (timers_)
		{
			player.addExtension(new PlayerFixesData(player, *timers_), true);
		}
	}

	bool sendGameTextToAll(StringView message, Milliseconds time, int style) override
	{
		// Check the parameters.
		if (!validateGameText(message, time, style))
		{
			return false;
		}
		for (auto player : players_->entries())
		{
			if (PlayerFixesData* data = queryExtension<PlayerFixesData>(player))
			{
				data->doSendGameText(message, time, style);
			}
		}
		// Always just return `true`.  What else should we return if half the
		// shows succeeded and half didn't?
		return true;
	}

	bool hideGameTextForAll(int style) override
	{
		for (auto player : players_->entries())
		{
			if (PlayerFixesData* data = queryExtension<PlayerFixesData>(player))
			{
				data->doHideGameText(style);
			}
		}
		// Always just return `true`.  What else should we return if half the
		// hides succeeded and half didn't?
		return true;
	}

	void clearAnimation(IPlayer* player, IActor* actor) override
	{
		if (player)
		{
			for (auto& anim : PlayerFixesData::animationToReapply_)
			{
				if (anim.player == player)
				{
					anim.player = nullptr;
				}
			}
		}
		if (actor)
		{
			for (auto& anim : PlayerFixesData::animationToReapply_)
			{
				if (anim.actor == actor)
				{
					anim.actor = nullptr;
				}
			}
		}
	}
};

COMPONENT_ENTRY_POINT()
{
	return new FixesComponent();
}
