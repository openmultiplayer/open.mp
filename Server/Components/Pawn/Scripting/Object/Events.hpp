#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct ObjectEvents : public ObjectEventHandler, public Singleton<ObjectEvents> {
	void onMoved(IObject& object) override {
		PawnManager::Get()->CallAllInSidesFirst("OnObjectMoved", object.getID());
	}

	void onPlayerObjectMoved(IPlayer& player, IPlayerObject& object) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerObjectMoved", object.getID());
	}

	void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditObject",
			player.getID(), 0, object.getID(), int(response),
			offset.x, offset.y, offset.z,
			rotation.x, rotation.y, rotation.z
		);
		if (!ret) {
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				player.getID(), 0, object.getID(), int(response),
				offset.x, offset.y, offset.z,
				rotation.x, rotation.y, rotation.z
			);
		}
	}

	void onPlayerObjectEdited(IPlayer& player, IPlayerObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditObject",
			player.getID(), 1, object.getID(), int(response),
			offset.x, offset.y, offset.z,
			rotation.x, rotation.y, rotation.z
		);
		if (!ret) {
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				player.getID(), 1, object.getID(), int(response),
				offset.x, offset.y, offset.z,
				rotation.x, rotation.y, rotation.z
			);
		}
	}

	void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerEditAttachedObject",
			player.getID(), saved, data.model, data.bone,
			data.offset.x, data.offset.y, data.offset.z,
			data.rotation.x, data.rotation.y, data.rotation.z,
			data.scale.x, data.scale.y, data.scale.z
		);
		if (!ret) {
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditAttachedObject",
				player.getID(), saved, data.model, data.bone,
				data.offset.x, data.offset.y, data.offset.z,
				data.rotation.x, data.rotation.y, data.rotation.z,
				data.scale.x, data.scale.y, data.scale.z
			);
		}
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerSelectObject",
			player.getID(), 1, object.getID(), model,
			position.x, position.y, position.z
		);
		if (!ret) {
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				player.getID(), 1, object.getID(), model,
				position.x, position.y, position.z
			);
		}
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override {
		cell ret = PawnManager::Get()->CallInSidesWhile0(
			"OnPlayerSelectObject",
			player.getID(), 2, object.getID(), model,
			position.x, position.y, position.z
		);
		if (!ret) {
			PawnManager::Get()->CallInEntry(
				"OnPlayerEditObject",
				player.getID(), 2, object.getID(), model,
				position.x, position.y, position.z
			);
		}
	}
};
