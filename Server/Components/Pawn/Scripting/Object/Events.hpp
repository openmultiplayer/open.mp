/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

struct ObjectEvents : public ObjectEventHandler, public Singleton<ObjectEvents>
{
	void onMoved(IObject& object) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnObjectMoved", DefaultReturnValue_True, object.getID());
	}

	void onPlayerObjectMoved(IPlayer& player, IPlayerObject& object) override
	{
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerObjectMoved", DefaultReturnValue_True, player.getID(), object.getID());
	}

	void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditObject",
			player.getID(), 0, object.getID(), int(response),
			offset.x, offset.y, offset.z,
			rotation.x, rotation.y, rotation.z);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				DefaultReturnValue_True,
				player.getID(), 0, object.getID(), int(response),
				offset.x, offset.y, offset.z,
				rotation.x, rotation.y, rotation.z);
		}
	}

	void onPlayerObjectEdited(IPlayer& player, IPlayerObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditObject",
			player.getID(), 1, object.getID(), int(response),
			offset.x, offset.y, offset.z,
			rotation.x, rotation.y, rotation.z);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				DefaultReturnValue_True,
				player.getID(), 1, object.getID(), int(response),
				offset.x, offset.y, offset.z,
				rotation.x, rotation.y, rotation.z);
		}
	}

	void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditAttachedObject",
			player.getID(), saved, index, data.model, data.bone,
			data.offset.x, data.offset.y, data.offset.z,
			data.rotation.x, data.rotation.y, data.rotation.z,
			data.scale.x, data.scale.y, data.scale.z);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditAttachedObject",
				DefaultReturnValue_True,
				player.getID(), saved, index, data.model, data.bone,
				data.offset.x, data.offset.y, data.offset.z,
				data.rotation.x, data.rotation.y, data.rotation.z,
				data.scale.x, data.scale.y, data.scale.z);
		}
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerSelectObject",
			player.getID(), 1, object.getID(), model,
			position.x, position.y, position.z);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry(
				"OnPlayerSelectObject",
				DefaultReturnValue_True,
				player.getID(), 1, object.getID(), model,
				position.x, position.y, position.z);
		}
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override
	{
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerSelectObject",
			player.getID(), 2, object.getID(), model,
			position.x, position.y, position.z);
		if (!ret)
		{
			PawnManager::Get()->CallInEntry(
				"OnPlayerSelectObject",
				DefaultReturnValue_True,
				player.getID(), 2, object.getID(), model,
				position.x, position.y, position.z);
		}
	}
};
