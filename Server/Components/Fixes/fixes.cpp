#include <Impl/events_impl.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

using namespace Impl;

class PlayerFixesData final : public IPlayerFixesData {
private:
	int lastCash_ = 0;

	int actorID_ = ACTOR_POOL_SIZE;
	IPlayer & player_;

	ITimer * killAll_;

public:
	void freeExtension() override
	{
		if (killAll_)
		{
			killAll_->kill();
		}
		delete this;
	}

	PlayerFixesData(IPlayer & player)
		: player_(player)
		, killAll_(nullptr)
	{
	}

	int getLastCash() const override
	{
		return lastCash_;
	}

	void setLastCash(int cash) override
	{
		lastCash_ = cash;
	}

	void createAnimatedActor(ITimersComponent * timers, char const * lib, char const * anim)
	{
		// Use the top of the actors range, to reduce the chance of conflicts.
		int actorID = --actorID_;

		NetCode::RPC::ShowActorForPlayer showActorForPlayerRPC;
		showActorForPlayerRPC.ActorID = actorID;
		showActorForPlayerRPC.Angle = 0.0f;
		showActorForPlayerRPC.Health = 1000.0f;
		showActorForPlayerRPC.Invulnerable = true;
		showActorForPlayerRPC.Position = { 0.0f, 0.0f, 0.0f };
		showActorForPlayerRPC.SkinID = 0;
		PacketHelper::send(showActorForPlayerRPC, player_);

		AnimationData animation {};
		animation.lib = lib;
		animation.name = anim;
		animation.loop = true;
		NetCode::RPC::ApplyActorAnimationForPlayer animationRPC(animation);
		animationRPC.ActorID = actorID;
		PacketHelper::send(animationRPC, player_);

		// Hide the actor later.
		if (killAll_ == nullptr)
		{
			killAll_ = timers->create(new SimpleTimerHandler([this]()
				{
					killAll_ = nullptr;
					while (this->actorID_ != ACTOR_POOL_SIZE)
					{
						NetCode::RPC::HideActorForPlayer RPC;
						RPC.ActorID = this->actorID_;
						PacketHelper::send(RPC, this->player_);
						++this->actorID_;
					}

				}), Milliseconds(350));
		}
	}

	// Inherited via IPlayerFixesData
	virtual void reset() override
	{
		lastCash_ = 0;
	}
};

class FixesComponent final : public IFixesComponent, public PlayerEventHandler, public ClassEventHandler {
private:
    ICore* core_ = nullptr;
	IClassesComponent * classes_ = nullptr;
	ITimersComponent * timers_ = nullptr;
	IPlayerPool * players_ = nullptr;

public:
    StringView componentName() const override
    {
        return "Fixes";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    FixesComponent()
    {
    }

	~FixesComponent()
	{
		if (core_)
		{
			//core->getEventDispatcher().removeEventHandler(this);
			players_->getEventDispatcher().removeEventHandler(this);
			classes_->getEventDispatcher().removeEventHandler(this);
		}
	}

	void free() override
	{
		delete this;
	}

    void onLoad(ICore* c) override
    {
		constexpr int8_t EventPriority_Fixes = -40;
		core_ = c;
		//core->getEventDispatcher().addEventHandler(this);
		players_ = &core_->getPlayers();
		players_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
	}

	void onInit(IComponentList * components) override
	{
		constexpr int8_t EventPriority_Fixes = -40;
		classes_ = components->queryComponent<IClassesComponent>();
		classes_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
		timers_ = components->queryComponent<ITimersComponent>();
	}

	void onPlayerSpawn(IPlayer & player) override
	{
		/*
		 * <problem>
		 *     San Andreas deducts $100 from players.
		 * </problem>
		 * <solution>
		 *     Reset the player's money to what it was before they died.
		 * </solution>
		 * <see>OnPlayerSpawn</see>
		 * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		 */
		IPlayerFixesData * data = queryExtension<IPlayerFixesData>(player);
		player.setMoney(data->getLastCash());
		data->setLastCash(0);
	}
	
	bool onPlayerRequestClass(IPlayer & player, unsigned int classId) override
	{
		/*
		 * <problem>
		 *     Random blunts and bottles sometimes appear in class selection.
		 * </problem>
		 * <solution>
		 *     Call "RemoveBuildingForPlayer".
		 * </solution>
		 * <see>OnPlayerRequestClass</see>
		 * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		 */
		auto pos = player.getPosition();
		player.removeDefaultObjects(1484, pos, 10.0f);
		player.removeDefaultObjects(1485, pos, 10.0f);
		player.removeDefaultObjects(1486, pos, 10.0f);
		return true;
	}

	void onPlayerDeath(IPlayer & player, IPlayer * killer, int reason) override
	{
		PlayerFixesData * data = queryExtension<PlayerFixesData>(player);
		data->setLastCash(player.getMoney());

		/*
		 * <problem>
		 *     Clients get stuck when they die with an animation applied.
		 * </problem>
		 * <solution>
		 *     Clear their animations.
		 * </solution>
		 * <see>OnPlayerDeath</see>
		 * <see>OnPlayerUpdate</see>
		 * <author    >h02</author>
		 * <post      href="https://sampforum.blast.hk/showthread.php?tid=312862&amp;pid=1641144#pid1641144" />
		 */
		auto anim = player.getAnimationData().name();
		if (anim.first.compare("PED"))
		{
			// Not in a "PED" library.  May get stuck.  Bypass the `ClearAnimations` fix.
			NetCode::RPC::ClearPlayerAnimations clearPlayerAnimationsRPC;
			clearPlayerAnimationsRPC.PlayerID = player.getID();
			PacketHelper::send(clearPlayerAnimationsRPC, player);
		}
	}

	void onPlayerConnect(IPlayer & player) override
	{
		player.addExtension(new PlayerFixesData(player), true);
		PlayerFixesData * data = queryExtension<PlayerFixesData>(player);

		/*
		 * <problem>
		 *     First time a library is used, it does nothing.
		 * </problem>
		 * <solution>
		 *     Preload them all, using some fake actors.
		 * </solution>
		 * <see>FIXES_ApplyAnimation</see>
		 * <author    href="https://github.com/simonepri/" >simonepri</author>
		 * <author    href="https://github.com/Lordzy/" >Lordzy</author>
		 * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		 */
		data->createAnimatedActor(timers_, "AIRPORT", "THRW_BARL_THRW");
		data->createAnimatedActor(timers_, "ATTRACTORS", "STEPSIT_IN");
		data->createAnimatedActor(timers_, "BAR", "BARCUSTOM_GET");
		data->createAnimatedActor(timers_, "BASEBALL", "BAT_1");
		data->createAnimatedActor(timers_, "BD_FIRE", "BD_FIRE1");
		data->createAnimatedActor(timers_, "BEACH", "BATHER");
		data->createAnimatedActor(timers_, "BENCHPRESS", "GYM_BP_CELEBRATE");
		data->createAnimatedActor(timers_, "BF_INJECTION", "BF_GETIN_LHS");
		data->createAnimatedActor(timers_, "BIKED", "BIKED_BACK");
		data->createAnimatedActor(timers_, "BIKEH", "BIKEH_BACK");
		data->createAnimatedActor(timers_, "BIKELEAP", "BK_BLNCE_IN");
		data->createAnimatedActor(timers_, "BIKES", "BIKES_BACK");
		data->createAnimatedActor(timers_, "BIKEV", "BIKEV_BACK");
		data->createAnimatedActor(timers_, "BIKE_DBZ", "PASS_DRIVEBY_BWD");
		data->createAnimatedActor(timers_, "BMX", "BMX_BACK");
		data->createAnimatedActor(timers_, "BOMBER", "BOM_PLANT");
		data->createAnimatedActor(timers_, "BOX", "BOXHIPIN");
		data->createAnimatedActor(timers_, "BSKTBALL", "BBALL_DEF_JUMP_SHOT");
		data->createAnimatedActor(timers_, "BUDDY", "BUDDY_CROUCHFIRE");
		data->createAnimatedActor(timers_, "BUS", "BUS_CLOSE");
		data->createAnimatedActor(timers_, "CAMERA", "CAMCRCH_CMON");
		data->createAnimatedActor(timers_, "CAR", "FIXN_CAR_LOOP");
		data->createAnimatedActor(timers_, "CARRY", "CRRY_PRTIAL");
		data->createAnimatedActor(timers_, "CAR_CHAT", "CARFONE_IN");
		data->createAnimatedActor(timers_, "CASINO", "CARDS_IN");
		data->createAnimatedActor(timers_, "CHAINSAW", "CSAW_1");
		data->createAnimatedActor(timers_, "CHOPPA", "CHOPPA_BACK");
		data->createAnimatedActor(timers_, "CLOTHES", "CLO_BUY");
		data->createAnimatedActor(timers_, "COACH", "COACH_INL");
		data->createAnimatedActor(timers_, "COLT45", "2GUNS_CROUCHFIRE");
		data->createAnimatedActor(timers_, "COP_AMBIENT", "COPBROWSE_IN");
		data->createAnimatedActor(timers_, "COP_DVBYZ", "COP_DVBY_B");
		data->createAnimatedActor(timers_, "CRACK", "BBALBAT_IDLE_01");
		data->createAnimatedActor(timers_, "CRIB", "CRIB_CONSOLE_LOOP");
		data->createAnimatedActor(timers_, "DAM_JUMP", "DAM_DIVE_LOOP");
		data->createAnimatedActor(timers_, "DANCING", "BD_CLAP");
		data->createAnimatedActor(timers_, "DILDO", "DILDO_1");
		data->createAnimatedActor(timers_, "DODGE", "COVER_DIVE_01");
		data->createAnimatedActor(timers_, "DOZER", "DOZER_ALIGN_LHS");
		data->createAnimatedActor(timers_, "DRIVEBYS", "GANG_DRIVEBYLHS");
		data->createAnimatedActor(timers_, "FAT", "FATIDLE");
		data->createAnimatedActor(timers_, "FIGHT_B", "FIGHTB_1");
		data->createAnimatedActor(timers_, "FIGHT_C", "FIGHTC_1");
		data->createAnimatedActor(timers_, "FIGHT_D", "FIGHTD_1");
		data->createAnimatedActor(timers_, "FIGHT_E", "FIGHTKICK");
		data->createAnimatedActor(timers_, "FINALE", "FIN_CLIMB_IN");
		data->createAnimatedActor(timers_, "FINALE2", "FIN_COP1_CLIMBOUT");
		data->createAnimatedActor(timers_, "FLAME", "FLAME_FIRE");
		data->createAnimatedActor(timers_, "FLOWERS", "FLOWER_ATTACK");
		data->createAnimatedActor(timers_, "FOOD", "EAT_BURGER");
		data->createAnimatedActor(timers_, "FREEWEIGHTS", "GYM_BARBELL");
		data->createAnimatedActor(timers_, "GANGS", "DEALER_DEAL");
		data->createAnimatedActor(timers_, "GHANDS", "GSIGN1");
		data->createAnimatedActor(timers_, "GHETTO_DB", "GDB_CAR2_PLY");
		data->createAnimatedActor(timers_, "GOGGLES", "GOGGLES_PUT_ON");
		data->createAnimatedActor(timers_, "GRAFFITI", "GRAFFITI_CHKOUT");
		data->createAnimatedActor(timers_, "GRAVEYARD", "MRNF_LOOP");
		data->createAnimatedActor(timers_, "GRENADE", "WEAPON_START_THROW");
		data->createAnimatedActor(timers_, "GYMNASIUM", "GYMSHADOWBOX");
		data->createAnimatedActor(timers_, "HAIRCUTS", "BRB_BEARD_01");
		data->createAnimatedActor(timers_, "HEIST9", "CAS_G2_GASKO");
		data->createAnimatedActor(timers_, "INT_HOUSE", "BED_IN_L");
		data->createAnimatedActor(timers_, "INT_OFFICE", "FF_DAM_FWD");
		data->createAnimatedActor(timers_, "INT_SHOP", "SHOP_CASHIER");
		data->createAnimatedActor(timers_, "JST_BUISNESS", "GIRL_01");
		data->createAnimatedActor(timers_, "KART", "KART_GETIN_LHS");
		data->createAnimatedActor(timers_, "KISSING", "BD_GF_WAVE");
		data->createAnimatedActor(timers_, "KNIFE", "KILL_KNIFE_PED_DAMAGE");
		data->createAnimatedActor(timers_, "LAPDAN1", "LAPDAN_D");
		data->createAnimatedActor(timers_, "LAPDAN2", "LAPDAN_D");
		data->createAnimatedActor(timers_, "LAPDAN3", "LAPDAN_D");
		data->createAnimatedActor(timers_, "LOWRIDER", "F_SMKLEAN_LOOP");
		data->createAnimatedActor(timers_, "MD_CHASE", "CARHIT_HANGON");
		data->createAnimatedActor(timers_, "MD_END", "END_SC1_PLY");
		data->createAnimatedActor(timers_, "MEDIC", "CPR");
		data->createAnimatedActor(timers_, "MISC", "BITCHSLAP");
		data->createAnimatedActor(timers_, "MTB", "MTB_BACK");
		data->createAnimatedActor(timers_, "MUSCULAR", "MSCLEWALKST_ARMED");
		data->createAnimatedActor(timers_, "NEVADA", "NEVADA_GETIN");
		data->createAnimatedActor(timers_, "ON_LOOKERS", "LKAROUND_IN");
		data->createAnimatedActor(timers_, "OTB", "BETSLP_IN");
		data->createAnimatedActor(timers_, "PARACHUTE", "FALL_SKYDIVE");
		data->createAnimatedActor(timers_, "PARK", "TAI_CHI_IN");
		data->createAnimatedActor(timers_, "PAULNMAC", "PISS_IN");
		data->createAnimatedActor(timers_, "PED", "ABSEIL");
		data->createAnimatedActor(timers_, "PLAYER_DVBYS", "PLYR_DRIVEBYBWD");
		data->createAnimatedActor(timers_, "PLAYIDLES", "SHIFT");
		data->createAnimatedActor(timers_, "POLICE", "COPTRAF_AWAY");
		data->createAnimatedActor(timers_, "POOL", "POOL_CHALKCUE");
		data->createAnimatedActor(timers_, "POOR", "WINWASH_START");
		data->createAnimatedActor(timers_, "PYTHON", "PYTHON_CROUCHFIRE");
		data->createAnimatedActor(timers_, "QUAD", "QUAD_BACK");
		data->createAnimatedActor(timers_, "QUAD_DBZ", "PASS_DRIVEBY_BWD");
		data->createAnimatedActor(timers_, "RAPPING", "LAUGH_01");
		data->createAnimatedActor(timers_, "RIFLE", "RIFLE_CROUCHFIRE");
		data->createAnimatedActor(timers_, "RIOT", "RIOT_ANGRY");
		data->createAnimatedActor(timers_, "ROB_BANK", "CAT_SAFE_END");
		data->createAnimatedActor(timers_, "ROCKET", "IDLE_ROCKET");
		data->createAnimatedActor(timers_, "RUSTLER", "PLANE_ALIGN_LHS");
		data->createAnimatedActor(timers_, "RYDER", "RYD_BECKON_01");
		data->createAnimatedActor(timers_, "SCRATCHING", "SCDLDLP");
		data->createAnimatedActor(timers_, "SHAMAL", "SHAMAL_ALIGN");
		data->createAnimatedActor(timers_, "SHOP", "ROB_2IDLE");
		data->createAnimatedActor(timers_, "SHOTGUN", "SHOTGUN_CROUCHFIRE");
		data->createAnimatedActor(timers_, "SILENCED", "CROUCHRELOAD");
		data->createAnimatedActor(timers_, "SKATE", "SKATE_IDLE");
		data->createAnimatedActor(timers_, "SMOKING", "F_SMKLEAN_LOOP");
		data->createAnimatedActor(timers_, "SNIPER", "WEAPON_SNIPER");
		data->createAnimatedActor(timers_, "SPRAYCAN", "SPRAYCAN_FIRE");
		data->createAnimatedActor(timers_, "STRIP", "PLY_CASH");
		data->createAnimatedActor(timers_, "SUNBATHE", "BATHERDOWN");
		data->createAnimatedActor(timers_, "SWAT", "GNSTWALL_INJURD");
		data->createAnimatedActor(timers_, "SWEET", "HO_ASS_SLAPPED");
		data->createAnimatedActor(timers_, "SWIM", "SWIM_BREAST");
		data->createAnimatedActor(timers_, "SWORD", "SWORD_1");
		data->createAnimatedActor(timers_, "TANK", "TANK_ALIGN_LHS");
		data->createAnimatedActor(timers_, "TATTOOS", "TAT_ARML_IN_O");
		data->createAnimatedActor(timers_, "TEC", "TEC_CROUCHFIRE");
		data->createAnimatedActor(timers_, "TRAIN", "TRAN_GTUP");
		data->createAnimatedActor(timers_, "TRUCK", "TRUCK_ALIGN_LHS");
		data->createAnimatedActor(timers_, "UZI", "UZI_CROUCHFIRE");
		data->createAnimatedActor(timers_, "VAN", "VAN_CLOSE_BACK_LHS");
		data->createAnimatedActor(timers_, "VENDING", "VEND_DRINK2_P");
		data->createAnimatedActor(timers_, "VORTEX", "CAR_JUMPIN_LHS");
		data->createAnimatedActor(timers_, "WAYFARER", "WF_BACK");
		data->createAnimatedActor(timers_, "WEAPONS", "SHP_1H_LIFT");
		data->createAnimatedActor(timers_, "WUZI", "CS_DEAD_GUY");
		data->createAnimatedActor(timers_, "WOP", "DANCE_G1");
		data->createAnimatedActor(timers_, "GFUNK", "DANCE_G1");
		data->createAnimatedActor(timers_, "RUNNINGMAN", "DANCE_G1");
		data->createAnimatedActor(timers_, "SAMP", "FISHINGIDLE");
		// This is last because it is the library used in the test suite.  If that works while being
		// loaded last all the others should load as well.
		data->createAnimatedActor(timers_, "DEALER", "DEALER_DEAL");
	}

	// Inherited via IFixesComponent
	virtual void reset() override
	{
	}
};

COMPONENT_ENTRY_POINT()
{
    return new FixesComponent();
}

