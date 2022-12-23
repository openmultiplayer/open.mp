/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "textlabel.hpp"
#include <Impl/pool_impl.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

using namespace Impl;

class PlayerTextLabelData final : public IPlayerTextLabelData
{
private:
	IPlayer& player;
	MarkedPoolStorage<PlayerTextLabel, IPlayerTextLabel, 0, TEXT_LABEL_POOL_SIZE> storage;

public:
	PlayerTextLabelData(IPlayer& player)
		: player(player)
	{
	}

	PlayerTextLabel* createInternal(StringView text, Colour colour, Vector3 pos, float drawDist, bool los)
	{
		return storage.emplace(player, text, colour, pos, drawDist, los);
	}

	IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) override
	{
		PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
		if (created)
		{
			created->streamInForClient(player, true);
		}
		return created;
	}

	IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IPlayer& attach) override
	{
		PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
		if (created)
		{
			created->attachToPlayer(attach, pos);
		}
		return created;
	}

	IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IVehicle& attach) override
	{
		PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
		if (created)
		{
			created->attachToVehicle(attach, pos);
		}
		return created;
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		storage.clear();
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	IPlayerTextLabel* get(int index) override
	{
		return storage.get(index);
	}

	void release(int index) override
	{
		auto ptr = storage.get(index);
		if (ptr)
		{
			ptr->destream();
			storage.release(index, false);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	/// Get a set of all the available labels
	const FlatPtrHashSet<IPlayerTextLabel>& entries() override
	{
		return storage._entries();
	}

	IEventDispatcher<PoolEventHandler<IPlayerTextLabel>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}
};

class TextLabelsComponent final : public ITextLabelsComponent, public PlayerConnectEventHandler, public PlayerUpdateEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	MarkedPoolStorage<TextLabel, ITextLabel, 0, TEXT_LABEL_POOL_SIZE> storage;
	IVehiclesComponent* vehicles = nullptr;
	IPlayerPool* players = nullptr;
	StreamConfigHelper streamConfigHelper;

public:
	StringView componentName() const override
	{
		return "TextLabels";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		players = &core->getPlayers();
		players->getPlayerUpdateDispatcher().addEventHandler(this);
		players->getPlayerConnectDispatcher().addEventHandler(this);
		players->getPoolEventDispatcher().addEventHandler(this);
		streamConfigHelper = StreamConfigHelper(core->getConfig());
	}

	void onInit(IComponentList* components) override
	{
		vehicles = components->queryComponent<IVehiclesComponent>();
	}

	~TextLabelsComponent()
	{
		if (core)
		{
			players->getPlayerUpdateDispatcher().removeEventHandler(this);
			players->getPlayerConnectDispatcher().removeEventHandler(this);
			players->getPoolEventDispatcher().removeEventHandler(this);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerTextLabelData(player), true);
	}

	ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los) override
	{
		ITextLabel* created = storage.emplace(text, colour, pos, drawDist, vw, los);

		if (created)
		{
			const float maxDist = streamConfigHelper.getDistanceSqr();

			for (IPlayer* player : players->entries())
			{
				updateLabelStateForPlayer(static_cast<TextLabel*>(created), *player, maxDist);
			}
		}
		return created;
	}

	ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IPlayer& attach) override
	{
		ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
		if (created)
		{
			created->attachToPlayer(attach, pos);
		}
		return created;
	}

	ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IVehicle& attach) override
	{
		ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
		if (created)
		{
			created->attachToVehicle(attach, pos);
		}
		return created;
	}

	void free() override
	{
		delete this;
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	ITextLabel* get(int index) override
	{
		return storage.get(index);
	}

	void release(int index) override
	{
		auto ptr = storage.get(index);
		if (ptr)
		{
			static_cast<TextLabel*>(ptr)->destream();
			storage.release(index, false);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	/// Get a set of all the available labels
	const FlatPtrHashSet<ITextLabel>& entries() override
	{
		return storage._entries();
	}

	IEventDispatcher<PoolEventHandler<ITextLabel>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		const float maxDist = streamConfigHelper.getDistanceSqr();
		if (streamConfigHelper.shouldStream(player.getID(), now))
		{
			for (ITextLabel* textLabel : storage)
			{
				updateLabelStateForPlayer(static_cast<TextLabel*>(textLabel), player, maxDist);
			}
		}

		return true;
	}

	void updateLabelStateForPlayer(TextLabel* label, IPlayer& player, float maxDist)
	{
		const TextLabelAttachmentData& data = label->getAttachmentData();
		Vector3 pos = label->getPosition();
		const int world = label->getVirtualWorld();
		bool worldOrAttached = player.getVirtualWorld() == world || world == -1;

		IPlayer* textLabelPlayer = players->get(data.playerID);
		if (textLabelPlayer)
		{
			worldOrAttached = textLabelPlayer->isStreamedInForPlayer(player);
			pos = textLabelPlayer->getPosition();
		}
		else if (vehicles)
		{
			IVehicle* textLabelVehicle = vehicles->get(data.vehicleID);
			if (textLabelVehicle)
			{
				worldOrAttached = textLabelVehicle->isStreamedInForPlayer(player);
				pos = textLabelVehicle->getPosition();
			}
		}

		const PlayerState state = player.getState();
		const Vector3 dist3D = pos - player.getPosition();
		const bool shouldBeStreamedIn = state != PlayerState_None && worldOrAttached && glm::dot(dist3D, dist3D) < maxDist;

		const bool isStreamedIn = label->isStreamedInForPlayer(player);
		if (!isStreamedIn && shouldBeStreamedIn)
		{
			label->streamInForPlayer(player);
		}
		else if (isStreamedIn && !shouldBeStreamedIn)
		{
			label->streamOutForPlayer(player);
		}
	}

	void onPoolEntryDestroyed(IPlayer& player) override
	{
		const int pid = player.getID();
		for (ITextLabel* textLabel : storage)
		{
			TextLabel* label = static_cast<TextLabel*>(textLabel);
			if (label->getAttachmentData().playerID == pid)
			{
				textLabel->detachFromPlayer(label->getPosition());
			}
			label->removeFor(pid, player);
		}
		for (IPlayer* player : players->entries())
		{
			IPlayerTextLabelData* data = queryExtension<IPlayerTextLabelData>(player);
			if (data)
			{
				for (IPlayerTextLabel* textLabel : *data)
				{
					PlayerTextLabel* label = static_cast<PlayerTextLabel*>(textLabel);
					if (label->getAttachmentData().playerID == pid)
					{
						textLabel->detachFromPlayer(label->getPosition());
					}
				}
			}
		}
	}

	void reset() override
	{
		// Destroy all stored entity instances.
		storage.clear();
	}
};

COMPONENT_ENTRY_POINT()
{
	return new TextLabelsComponent();
}
