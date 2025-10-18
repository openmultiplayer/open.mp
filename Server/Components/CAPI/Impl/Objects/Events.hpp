/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
struct ObjectEvents : public ObjectEventHandler, public Singleton<ObjectEvents<PRIORITY>>
{
	void onMoved(IObject& object) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onObjectMove", EventReturnHandler::None, &object);
	}

	void onPlayerObjectMoved(IPlayer& player, IPlayerObject& object) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerObjectMove", EventReturnHandler::None, &player, &object);
	}

	void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEditObject", EventReturnHandler::None, &player, &object, int(response), offset.x, offset.y, offset.z, rotation.x, rotation.y, rotation.z);
	}

	void onPlayerObjectEdited(IPlayer& player, IPlayerObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEditPlayerObject", EventReturnHandler::None, &player, &object, int(response), offset.x, offset.y, offset.z, rotation.x, rotation.y, rotation.z);
	}

	void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEditAttachedObject", EventReturnHandler::None,
			&player, saved, index, data.model, data.bone,
			data.offset.x, data.offset.y, data.offset.z,
			data.rotation.x, data.rotation.y, data.rotation.z,
			data.scale.x, data.scale.y, data.scale.z);
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerSelectObject", EventReturnHandler::None, &player, &object, model, position.x, position.y, position.z);
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerSelectPlayerObject", EventReturnHandler::None, &player, &object, model, position.x, position.y, position.z);
	}
};
