#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <fstream>

struct LegacyConfigComponent final : public IConfigProviderComponent {
	PROVIDE_UUID(0x24ef6216838f9ffc);

	StringView componentName() override {
		return "LegacyConfig";
	}

	bool configure(IEarlyConfig& config) override {
		if (config.getString("bot_exe").empty()) {
			config.setString("bot_exe", "samp-npc");
		}

		std::ifstream ifs("samp.ban");
		if (ifs.good()) {
			for (String line; std::getline(ifs, line);) {
				size_t first = line.find_first_of(' ');
				if (first != -1) {
					PeerAddress address;
					address.ipv6 = false;
					if (PeerAddress::FromString(address, line.substr(0, first))) {
						config.addBan(BanEntry(address, "LEGACY", "LEGACY"));
					}
				}
			}
		}

		// todo read server.cfg, process it
		return true;
	}

} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
