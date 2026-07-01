/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(TextLabel_Create, objectPtr(StringCharPtr text, uint32_t color, float x, float y, float z, float drawDistance, int virtualWorld, bool los, int* id))
{
	ITextLabelsComponent* component = ComponentManager::Get()->textlabels;
	if (component)
	{
		ITextLabel* textlabel = component->create(text, Colour::FromRGBA(color), { x, y, z }, drawDistance, virtualWorld, los);
		if (textlabel)
		{
			*id = textlabel->getID();
			return textlabel;
		}
	}
	return nullptr;
}

OMP_CAPI(TextLabel_Destroy, bool(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	ComponentManager::Get()->textlabels->release(textlabel_->getID());
	return true;
}

OMP_CAPI(TextLabel_FromID, objectPtr(int textlabelid))
{
	ITextLabelsComponent* component = ComponentManager::Get()->textlabels;
	if (component)
	{
		return component->get(textlabelid);
	}
	return nullptr;
}

OMP_CAPI(TextLabel_GetID, int(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	return textlabel_->getID();
}

OMP_CAPI(TextLabel_AttachToPlayer, bool(objectPtr textlabel, objectPtr player, float offsetX, float offsetY, float offsetZ))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	textlabel_->attachToPlayer(*player_, { offsetX, offsetY, offsetZ });
	return true;
}

OMP_CAPI(TextLabel_AttachToVehicle, bool(objectPtr textlabel, objectPtr vehicle, float offsetX, float offsetY, float offsetZ))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	textlabel_->attachToVehicle(*vehicle_, { offsetX, offsetY, offsetZ });
	return true;
}

OMP_CAPI(TextLabel_UpdateText, bool(objectPtr textlabel, uint32_t color, StringCharPtr text))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	textlabel_->setColourAndText(Colour::FromRGBA(color), text);
	return true;
}

OMP_CAPI(TextLabel_IsValid, bool(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	if (!textlabels->get(textlabel_->getID()))
		return false;
	return true;
}

OMP_CAPI(TextLabel_IsStreamedIn, bool(objectPtr player, objectPtr textlabel))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	auto streamed = textlabel_->isStreamedInForPlayer(*player_);
	return streamed;
}

OMP_CAPI(TextLabel_GetText, bool(objectPtr textlabel, OutputStringViewPtr output))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	auto result = textlabel_->getText();
	SET_CAPI_STRING_VIEW(output, result);
	return true;
}

OMP_CAPI(TextLabel_GetColor, uint32_t(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, 0);
	return textlabel_->getColour().RGBA();
}

OMP_CAPI(TextLabel_GetPos, bool(objectPtr textlabel, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	const Vector3& pos = textlabel_->getPosition();

	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(TextLabel_SetDrawDistance, bool(objectPtr textlabel, float distance))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	textlabel_->setDrawDistance(distance);
	return true;
}

OMP_CAPI(TextLabel_GetDrawDistance, float(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, 0.0f);
	auto distance = textlabel_->getDrawDistance();
	return distance;
}

OMP_CAPI(TextLabel_GetLOS, bool(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	auto los = textlabel_->getTestLOS();
	return los;
}

OMP_CAPI(TextLabel_SetLOS, bool(objectPtr textlabel, bool status))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	textlabel_->setTestLOS(status);
	return true;
}

OMP_CAPI(TextLabel_GetVirtualWorld, int(objectPtr textlabel))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, 0);
	auto virtualWorld = textlabel_->getVirtualWorld();
	return virtualWorld;
}

OMP_CAPI(TextLabel_SetVirtualWorld, bool(objectPtr textlabel, int world))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	textlabel_->setVirtualWorld(world);
	return true;
}

OMP_CAPI(TextLabel_GetAttachedData, bool(objectPtr textlabel, int* attached_player, int* attached_vehicle))
{
	POOL_ENTITY_RET(textlabels, ITextLabel, textlabel, textlabel_, false);
	const TextLabelAttachmentData& data = textlabel_->getAttachmentData();

	*attached_player = data.playerID;
	*attached_vehicle = data.vehicleID;

	return true;
}

/*
	Per-Player TextLabel
*/

OMP_CAPI(PlayerTextLabel_Create, objectPtr(objectPtr player, StringCharPtr text, uint32_t color, float x, float y, float z, float drawDistance, objectPtr attachedPlayer, objectPtr attachedVehicle, bool los, int* id))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerTextLabelData* labelData = queryExtension<IPlayerTextLabelData>(player_);
	if (labelData)
	{
		IPlayerTextLabel* textlabel = nullptr;

		if (attachedPlayer)
		{
			textlabel = labelData->create(text, Colour::FromRGBA(color), { x, y, z }, drawDistance, los, *reinterpret_cast<IPlayer*>(attachedPlayer));
		}
		else if (attachedVehicle)
		{
			textlabel = labelData->create(text, Colour::FromRGBA(color), { x, y, z }, drawDistance, los, *reinterpret_cast<IVehicle*>(attachedVehicle));
		}
		else
		{
			textlabel = labelData->create(text, Colour::FromRGBA(color), { x, y, z }, drawDistance, los);
		}

		if (textlabel)
		{
			*id = textlabel->getID();
			return textlabel;
		}
	}
	return nullptr;
}

OMP_CAPI(PlayerTextLabel_Destroy, bool(objectPtr player, objectPtr textlabel))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	auto data = queryExtension<IPlayerTextLabelData>(player_);
	if (!data)
	{
		return false;
	}
	data->release(textlabel_->getID());
	return true;
}

OMP_CAPI(PlayerTextLabel_FromID, objectPtr(objectPtr player, int textlabelid))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerTextLabelData* labelData = queryExtension<IPlayerTextLabelData>(player_);
	if (labelData)
	{
		return labelData->get(textlabelid);
	}
	return nullptr;
}

OMP_CAPI(PlayerTextLabel_GetID, int(objectPtr player, objectPtr textlabel))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	return textlabel_->getID();
}

OMP_CAPI(PlayerTextLabel_UpdateText, bool(objectPtr player, objectPtr textlabel, uint32_t color, StringCharPtr text))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	textlabel_->setColourAndText(Colour::FromRGBA(color), text);
	return true;
}

OMP_CAPI(PlayerTextLabel_IsValid, bool(objectPtr player, objectPtr textlabel, bool* valid))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	if (!playerData->get(textlabel_->getID()))
		return false;
	return true;
}

OMP_CAPI(PlayerTextLabel_GetText, bool(objectPtr player, objectPtr textlabel, OutputStringViewPtr output))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	auto result = textlabel_->getText();
	SET_CAPI_STRING_VIEW(output, result);
	return true;
}

OMP_CAPI(PlayerTextLabel_GetColor, bool(objectPtr player, objectPtr textlabel, uint32_t* color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	*color = textlabel_->getColour().RGBA();
	return true;
}

OMP_CAPI(PlayerTextLabel_GetPos, bool(objectPtr player, objectPtr textlabel, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	const Vector3& pos = textlabel_->getPosition();

	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(PlayerTextLabel_SetDrawDistance, bool(objectPtr player, objectPtr textlabel, float distance))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	textlabel_->setDrawDistance(distance);
	return true;
}

OMP_CAPI(PlayerTextLabel_GetDrawDistance, float(objectPtr player, objectPtr textlabel))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, 0.0f);
	auto distance = textlabel_->getDrawDistance();
	return distance;
}

OMP_CAPI(PlayerTextLabel_GetLOS, bool(objectPtr player, objectPtr textlabel))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	auto los = textlabel_->getTestLOS();
	return los;
}

OMP_CAPI(PlayerTextLabel_SetLOS, bool(objectPtr player, objectPtr textlabel, bool status))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	textlabel_->setTestLOS(status);
	return true;
}

OMP_CAPI(PlayerTextLabel_GetVirtualWorld, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	return player_->getVirtualWorld();
}

OMP_CAPI(PlayerTextLabel_GetAttachedData, bool(objectPtr player, objectPtr textlabel, int* attached_player, int* attached_vehicle))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextLabelData, IPlayerTextLabel, textlabel, textlabel_, false);
	const TextLabelAttachmentData& data = textlabel_->getAttachmentData();

	*attached_player = data.playerID;
	*attached_vehicle = data.vehicleID;

	return true;
}
