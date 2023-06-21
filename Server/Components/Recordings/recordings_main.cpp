/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Recordings/recordings.hpp>
#include <sdk.hpp>
#include <netcode.hpp>
#include <ghc/filesystem.hpp>

class PlayerRecordingData final : public IPlayerRecordingData
{
private:
	PlayerRecordingType type_ = PlayerRecordingType_None;
	TimePoint start_ = TimePoint();
	std::ofstream file_;

	friend class RecordingsComponent;

public:
	void start(PlayerRecordingType type, StringView file) override
	{
		type_ = type;
		start_ = Time::now();

		ghc::filesystem::path scriptfilesPath = ghc::filesystem::absolute("scriptfiles");
		if (!ghc::filesystem::exists(scriptfilesPath) || !ghc::filesystem::is_directory(scriptfilesPath))
		{
			ghc::filesystem::create_directory(scriptfilesPath);
		}
		auto filePath = scriptfilesPath / ghc::filesystem::path(std::string(file) + ".rec");
		file_.open(filePath.string(), std::ios_base::out | std::ios_base::binary);

		// Write recording header
		if (file_.good())
		{
			uint32_t version = 1000;
			file_.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));
			file_.write(reinterpret_cast<const char*>(&type_), sizeof(uint32_t));
		}

		// To view/edit the recorded data as a CSV, see https://github.com/WoutProvost/samp-rec-to-csv

		// SA-MP server:
		// - file already exists ==> overwrite
		// - mode is NONE ==>  don't record parts when on foot or driver, so only header is written
		// - mode is DRIVER ==> don't record parts when on foot, but record parts when driver, timer keeps going for non recorded parts
		// - mode is ONFOOT ==> don't record parts when driver, but record parts when on foot, timer keeps going for non recorded parts
		// - OnPlayerUpdate return 0 ==> keep recording, recording not affected by this event's return value
	}

	void stop() override
	{
		type_ = PlayerRecordingType_None;
		start_ = TimePoint();
		file_.close();
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		stop();
	}
};

class RecordingsComponent final : public IRecordingsComponent, public PlayerConnectEventHandler
{
private:
	ICore* core = nullptr;

	struct OnFootRecordingHandler : public SingleNetworkInEventHandler
	{
		RecordingsComponent& self;
		OnFootRecordingHandler(RecordingsComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			PlayerRecordingData* data = queryExtension<PlayerRecordingData>(peer);
			if (!data)
			{
				return false;
			}

			// Write on foot recording data
			if (data->type_ == PlayerRecordingType_Driver && data->file_.good())
			{
				const uint32_t timeSinceRecordStart = duration_cast<Milliseconds>(Time::now() - data->start_).count();
				data->file_.write(reinterpret_cast<const char*>(&timeSinceRecordStart), sizeof(uint32_t));
				data->file_.write(reinterpret_cast<const char*>(bs.GetData()), bs.GetNumberOfBytesUsed());
			}

			return true;
		}
	} onFootRecordingHandler;

	struct DriverRecordingHandler : public SingleNetworkInEventHandler
	{
		RecordingsComponent& self;
		DriverRecordingHandler(RecordingsComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			PlayerRecordingData* data = queryExtension<PlayerRecordingData>(peer);
			if (!data)
			{
				return false;
			}

			// Write driver recording data
			if (data->type_ == PlayerRecordingType_Driver && data->file_.good())
			{
				uint32_t timeSinceRecordStart = duration_cast<Milliseconds>(Time::now() - data->start_).count();
				data->file_.write(reinterpret_cast<const char*>(&timeSinceRecordStart), sizeof(uint32_t));
				data->file_.write(reinterpret_cast<const char*>(bs.GetData()), bs.GetNumberOfBytesUsed());
			}

			return true;
		}
	} driverRecordingHandler;

public:
	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerRecordingData(), true);
	}

	StringView componentName() const override
	{
		return "Recordings";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	RecordingsComponent()
		: onFootRecordingHandler(*this)
		, driverRecordingHandler(*this)
	{
	}

	void onLoad(ICore* c) override
	{
		core = c;
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		NetCode::Packet::PlayerFootSync::addEventHandler(*core, &onFootRecordingHandler);
		NetCode::Packet::PlayerVehicleSync::addEventHandler(*core, &driverRecordingHandler);
	}

	void reset() override
	{
		for (IPlayer* player : core->getPlayers().entries())
		{
			if (player)
			{
				IPlayerRecordingData* recording = queryExtension<IPlayerRecordingData>(player);
				if (recording)
				{
					recording->stop();
				}
			}
		}
	}

	void free() override
	{
		delete this;
	}

	~RecordingsComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
			NetCode::Packet::PlayerFootSync::removeEventHandler(*core, &onFootRecordingHandler);
			NetCode::Packet::PlayerVehicleSync::removeEventHandler(*core, &driverRecordingHandler);
		}
	}
};

COMPONENT_ENTRY_POINT()
{
	return new RecordingsComponent();
}
