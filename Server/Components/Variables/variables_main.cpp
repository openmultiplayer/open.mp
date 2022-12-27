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

char ascii_toupper_char(char c)
{
	return ('a' <= c && c <= 'z') ? c ^ 0x20 : c;
}

// Auto-vectorized toupper function
// You can find out why this is being used in here: https://stackoverflow.com/a/37151084
// Ours is probably going to be even faster since we don't have to use strlen;
// Because string_view::length() already holds string length
// TODO: Move this to somewhere more appropiate, we don't have shared utils yet.
size_t strtoupper(String& dst, const StringView& src)
{
	size_t length = src.length();
	dst.resize(length);
	for (size_t i = 0; i < length; ++i)
	{
		dst[i] = ascii_toupper_char(src[i]);
	}
	return length;
}

#define TO_UPPER_KEY(dest, source) \
	String dest;                   \
	strtoupper(dest, source);

template <class ToInherit>
class VariableStorageBase : public ToInherit
{
private:
	FlatHashMap<String, std::variant<int, String, float>> data_;

public:
	void setString(StringView key, StringView value) override
	{
		TO_UPPER_KEY(upperKey, key);
		data_[upperKey].template emplace<String>(value);
	}

	const StringView getString(StringView key) const override
	{
		TO_UPPER_KEY(upperKey, key);
		auto it = data_.find(upperKey);
		if (it == data_.end())
		{
			return StringView();
		}
		if (it->second.index() != 1)
		{
			return StringView();
		}
		return StringView(std::get<String>(it->second));
	}

	void setInt(StringView key, int value) override
	{
		TO_UPPER_KEY(upperKey, key);
		data_[upperKey].template emplace<int>(value);
	}

	int getInt(StringView key) const override
	{
		TO_UPPER_KEY(upperKey, key);
		auto it = data_.find(upperKey);
		if (it == data_.end())
		{
			return 0;
		}
		if (it->second.index() != 0)
		{
			return 0;
		}
		return std::get<int>(it->second);
	}

	void setFloat(StringView key, float value) override
	{
		TO_UPPER_KEY(upperKey, key);
		data_[upperKey].template emplace<float>(value);
	}

	float getFloat(StringView key) const override
	{
		TO_UPPER_KEY(upperKey, key);
		auto it = data_.find(upperKey);
		if (it == data_.end())
		{
			return 0;
		}
		if (it->second.index() != 2)
		{
			return 0;
		}
		return std::get<float>(it->second);
	}

	VariableType getType(StringView key) const override
	{
		TO_UPPER_KEY(upperKey, key);
		auto it = data_.find(upperKey);
		if (it == data_.end())
		{
			return VariableType_None;
		}
		size_t index = it->second.index();
		if (index == std::variant_npos)
		{
			return VariableType_None;
		}
		return VariableType(index + 1);
	}

	bool erase(StringView key) override
	{
		TO_UPPER_KEY(upperKey, key);
		auto it = data_.find(upperKey);
		if (it == data_.end())
		{
			return false;
		}
		data_.erase(it);
		return true;
	}

	bool getKeyAtIndex(int index, StringView& key) const override
	{
		auto it = std::next(data_.begin(), index);
		if (it != data_.end())
		{
			key = it->first;
			return true;
		}
		return false;
	}

	int size() const override
	{
		return data_.size();
	}

	void clear()
	{
		data_.clear();
	}
};

class PlayerVariableData final : public VariableStorageBase<IPlayerVariableData>
{
public:
	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		clear();
	}
};

class VariablesComponent final : public VariableStorageBase<IVariablesComponent>, public PlayerConnectEventHandler
{
private:
	ICore* core = nullptr;

public:
	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerVariableData(), true);
	}

	StringView componentName() const override
	{
		return "Variables";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
	}

	void free() override
	{
		delete this;
	}

	~VariablesComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
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
