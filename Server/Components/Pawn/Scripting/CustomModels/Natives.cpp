/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"

SCRIPT_API(AddCharModel, bool(int baseid, int newid, std::string const& dff, std::string const& textureLibrary))
{
	auto models = PawnManager::Get()->models;

	if (!models)
		return false;

	return models->addCustomModel(ModelType::Skin, newid, baseid, dff, textureLibrary);
}

SCRIPT_API(AddSimpleModel, bool(int virtualWorld, int baseid, int newid, std::string const& dff, std::string const& textureLibrary))
{
	auto models = PawnManager::Get()->models;

	if (!models)
		return false;

	return models->addCustomModel(ModelType::Object, newid, baseid, dff, textureLibrary, virtualWorld);
}

SCRIPT_API(AddSimpleModelTimed, bool(int virtualWorld, int baseid, int newid, std::string const& dff, std::string const& textureLibrary, int timeOn, int timeOff))
{
	auto models = PawnManager::Get()->models;

	if (!models)
		return false;

	return models->addCustomModel(ModelType::Object, newid, baseid, dff, textureLibrary, virtualWorld, timeOn, timeOff);
}

SCRIPT_API(GetPlayerCustomSkin, int(IPlayer& player))
{
	IPlayerCustomModelsData* data = queryExtension<IPlayerCustomModelsData>(player);
	if (!data)
	{
		return 0;
	}
	return data->getCustomSkin();
}

SCRIPT_API(RedirectDownload, bool(IPlayer& player, std::string const& url))
{
	IPlayerCustomModelsData* data = queryExtension<IPlayerCustomModelsData>(player);
	if (!data)
	{
		return false;
	}
	if (!data->sendDownloadUrl(url))
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "This native can be used only within OnPlayerRequestDownload callback.");
		return false;
	}
	return true;
}

SCRIPT_API(FindModelFileNameFromCRC, bool(int crc, OutputOnlyString& output))
{
	auto models = PawnManager::Get()->models;

	if (!models)
	{
		return false;
	}

	output = models->getModelNameFromChecksum(crc);
	return std::get<StringView>(output).length();
}

SCRIPT_API(FindTextureFileNameFromCRC, bool(int crc, OutputOnlyString& output))
{
	return openmp_scripting::FindModelFileNameFromCRC(crc, output);
}

SCRIPT_API(IsValidCustomModel, bool(int modelId))
{
	auto models = PawnManager::Get()->models;

	if (!models)
	{
		return false;
	}

	return models->isValidCustomModel(modelId);
}

SCRIPT_API(GetCustomModelPath, bool(int modelId, OutputOnlyString& dffPath, OutputOnlyString& txdPath))
{
	auto models = PawnManager::Get()->models;

	if (!models)
	{
		return false;
	}

	StringView dffPathSV {};
	StringView txdPathSV {};

	auto status = models->getCustomModelPath(modelId, dffPathSV, txdPathSV);

	dffPath = dffPathSV;
	txdPath = txdPathSV;

	return status;
}
