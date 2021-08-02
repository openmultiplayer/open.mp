#include <Server/Components/Variables/variables.hpp>
#include <sdk.hpp>
#include <variant>
#include <unordered_map>
#include <absl/container/flat_hash_map.h>

template <class ToInherit>
struct VariableStorageBase : public ToInherit {
	void setString(StringView key, StringView value) override {
		data_[key].emplace<String>(value);
	}
	const StringView getString(StringView key) const override {
		auto it = data_.find(key);
		if (it == data_.end()) {
			return StringView();
		}
		if (it->second.index() != 1) {
			return StringView();
		}
		return StringView(std::get<String>(it->second));
	}

	void setInt(StringView key, int value) override {
		data_[key].emplace<int>(value);
	}

	int getInt(StringView key) const override {
		auto it = data_.find(key);
		if (it == data_.end()) {
			return 0;
		}
		if (it->second.index() != 0) {
			return 0;
		}
		return std::get<int>(it->second);
	}

	void setFloat(StringView key, float value) override {
		data_[key].emplace<float>(value);
	}

	float getFloat(StringView key) const override {
		auto it = data_.find(key);
		if (it == data_.end()) {
			return 0;
		}
		if (it->second.index() != 2) {
			return 0;
		}
		return std::get<float>(it->second);
	}

	VariableType getType(StringView key) const override {
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

	bool erase(StringView key) override {
		return data_.erase(key) == 1;
	}

private:
	FlatHashMap<String, Variant<int, String, float>> data_;
};

struct PlayerVariableData final : VariableStorageBase<IPlayerVariableData> {

	void free() override {
		delete this;
	}
};

struct VariablesComponent final : VariableStorageBase<IVariablesComponent>, PlayerEventHandler {
	ICore* core;

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerVariableData();
	}

	StringView componentName() override {
		return "Variables";
	}

	void onLoad(ICore * core) override {
		this->core = core;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
	}

	~VariablesComponent() {
		if (core) {
			core->getPlayers().getEventDispatcher().removeEventHandler(this);
		}
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
