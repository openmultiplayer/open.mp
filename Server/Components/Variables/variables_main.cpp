#include <Server/Components/Variables/variables.hpp>
#include <sdk.hpp>
#include <variant>
#include <unordered_map>

template <class ToInherit>
struct VariableStorageBase : public ToInherit {
	void setString(const String& k, const String& value) override {
		std::string key = k.c_str();
		data_[key].emplace<String>(value);
	}
	const unsigned int getString(const String& k, String& out) const override {
		std::string key = k.c_str();
		auto it = data_.find(key);
		if (it == data_.end()) {
			return 0;
		}
		if (it->second.index() != 1) {
			return 0;
		}
		out = std::get<String>(it->second);
		return out.length();
	}

	void setInt(const String& k, int value) override {
		std::string key = k.c_str();
		data_[key].emplace<int>(value);
	}

	int getInt(const String& k) const override {
		std::string key = k.c_str();
		auto it = data_.find(key);
		if (it == data_.end()) {
			return 0;
		}
		if (it->second.index() != 0) {
			return 0;
		}
		return std::get<int>(it->second);
	}

	void setFloat(const String& k, float value) override {
		std::string key = k.c_str();
		data_[key].emplace<float>(value);
	}

	float getFloat(const String& k) const override {
		std::string key = k.c_str();
		auto it = data_.find(key);
		if (it == data_.end()) {
			return 0;
		}
		if (it->second.index() != 2) {
			return 0;
		}
		return std::get<float>(it->second);
	}

	VariableType getType(const String& k) const override {
		std::string key = k.c_str();
		auto it = data_.find(key);
		if (it == data_.end()) {
			return VariableType_None;
		}
		size_t index = it->second.index();
		if (index == std::variant_npos) {
			return VariableType_None;
		}
		return VariableType(index + 1);
	}

	bool erase(const String& k) override {
		std::string key = k.c_str();
		return data_.erase(key) == 1;
	}

private:
	std::unordered_map<std::string, std::variant<int, String, float>> data_;
};

struct PlayerVariableData final : VariableStorageBase<IPlayerVariableData> {

	void free() override {
		delete this;
	}
};

struct VariablesPlugin final : VariableStorageBase<IVariablesPlugin>, PlayerEventHandler {
	ICore* core;

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerVariableData();
	}

	const char * pluginName() override {
		return "Variables";
	}

	void onInit(ICore * core) override {
		this->core = core;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
	}

	~VariablesPlugin() {
		if (core) {
			core->getPlayers().getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
