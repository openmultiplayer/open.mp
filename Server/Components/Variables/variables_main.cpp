/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Variables/variables.hpp>
#include <sdk.hpp>
#include <variant>

using namespace Impl;

template <class ToInherit>
class VariableStorageBase : public ToInherit {
private:
    FlatHashMap<String, std::variant<int, String, float>> data_;

public:
    void setString(StringView key, StringView value) override
    {
        data_[String(key)].emplace<String>(value);
    }

    const StringView getString(StringView key) const override
    {
        auto it = data_.find(String(key));
        if (it == data_.end()) {
            return StringView();
        }
        if (it->second.index() != 1) {
            return StringView();
        }
        return StringView(std::get<String>(it->second));
    }

    void setInt(StringView key, int value) override
    {
        data_[String(key)].emplace<int>(value);
    }

    int getInt(StringView key) const override
    {
        auto it = data_.find(String(key));
        if (it == data_.end()) {
            return 0;
        }
        if (it->second.index() != 0) {
            return 0;
        }
        return std::get<int>(it->second);
    }

    void setFloat(StringView key, float value) override
    {
        data_[String(key)].emplace<float>(value);
    }

    float getFloat(StringView key) const override
    {
        auto it = data_.find(String(key));
        if (it == data_.end()) {
            return 0;
        }
        if (it->second.index() != 2) {
            return 0;
        }
        return std::get<float>(it->second);
    }

    VariableType getType(StringView key) const override
    {
        auto it = data_.find(String(key));
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
        auto it = data_.find(String(key));
        if (it == data_.end()) {
            return false;
        }
        it->second = std::variant<int, String, float>();
        return true;
    }

    bool getKeyAtIndex(int index, StringView& key) const override
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
};

class PlayerVariableData final : public VariableStorageBase<IPlayerVariableData> {
public:
    void freeExtension() override
    {
        delete this;
    }

    void reset() override
    {
        // Do nothing.  Variables persist.
    }
};

class VariablesComponent final : public VariableStorageBase<IVariablesComponent>, public PlayerEventHandler {
private:
    ICore* core = nullptr;

public:
    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerVariableData(), true);
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

    void reset() override
    {
        // Nothing to reset here.  SVars persist.
    }
};

COMPONENT_ENTRY_POINT()
{
    return new VariablesComponent();
}
