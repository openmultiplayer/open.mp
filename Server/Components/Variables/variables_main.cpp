#include <Server/Components/Variables/variables.hpp>
#include <sdk.hpp>
#include <unordered_map>
#include <variant>

template <class ToInherit>
struct VariableStorageBase : public ToInherit {
    void setString(StringView key, StringView value) override
    {
        data_[key].emplace<String>(value);
    }
    const StringView getString(StringView key) const override
    {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return StringView();
        }
        if (it->second.index() != 1) {
            return StringView();
        }
        return StringView(variant_get<String>(it->second));
    }

    void setInt(StringView key, int value) override
    {
        data_[key].emplace<int>(value);
    }

    int getInt(StringView key) const override
    {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return 0;
        }
        if (it->second.index() != 0) {
            return 0;
        }
        return variant_get<int>(it->second);
    }

    void setFloat(StringView key, float value) override
    {
        data_[key].emplace<float>(value);
    }

    float getFloat(StringView key) const override
    {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return 0;
        }
        if (it->second.index() != 2) {
            return 0;
        }
        return variant_get<float>(it->second);
    }

    VariableType getType(StringView key) const override
    {
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

    bool erase(StringView key) override
    {
        auto it = data_.find(key);
        if (it == data_.end()) {
            return false;
        }
        it->second = Variant<int, String, float>();
        return true;
    }

    bool getKeyAtIndex(int index, StringView key) const override
    {
        auto it = std::next(data_.begin(), index);
        if (it != data_.end()) {
            key = it->first;
            return true;
        }
        return false;
    }

    int size() const override
    {
        return data_.size();
    }

private:
    FlatHashMap<String, Variant<int, String, float>> data_;
};

struct PlayerVariableData final : VariableStorageBase<IPlayerVariableData> {

    void free() override
    {
        delete this;
    }
};

struct VariablesComponent final : VariableStorageBase<IVariablesComponent>, PlayerEventHandler {
    ICore* core = nullptr;

    void onConnect(IPlayer& player) override
    {
        player.addData(new PlayerVariableData());
    }

    StringView componentName() const override
    {
        return "Variables";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
    }

    void free() override
    {
        delete this;
    }

    ~VariablesComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new VariablesComponent();
}
