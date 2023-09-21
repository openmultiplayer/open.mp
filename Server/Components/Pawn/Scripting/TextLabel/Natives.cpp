/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>
#include "../../format.hpp"

SCRIPT_API_FAILRET(Create3DTextLabel, INVALID_TEXT_LABEL_ID, int(cell const* format, uint32_t colour, Vector3 position, float drawDistance, int virtualWorld, bool los))
{
	ITextLabelsComponent* component = PawnManager::Get()->textlabels;
	if (component)
	{
		AmxStringFormatter text(format, GetAMX(), GetParams(), 8); // Not 6
		ITextLabel* textlabel = component->create(text, Colour::FromRGBA(colour), position, drawDistance, virtualWorld, los);
		if (textlabel)
		{
			return textlabel->getID();
		}
	}
	return FailRet;
}

SCRIPT_API(Delete3DTextLabel, bool(ITextLabel& textlabel))
{
	PawnManager::Get()->textlabels->release(textlabel.getID());
	return true;
}

SCRIPT_API(Attach3DTextLabelToPlayer, bool(ITextLabel& textlabel, IPlayer& player, Vector3 offset))
{
	textlabel.attachToPlayer(player, offset);
	return true;
}

SCRIPT_API(Attach3DTextLabelToVehicle, bool(ITextLabel& textlabel, IVehicle& vehicle, Vector3 offset))
{
	textlabel.attachToVehicle(vehicle, offset);
	return true;
}

SCRIPT_API(Update3DTextLabelText, bool(ITextLabel& textlabel, uint32_t colour, cell const* format))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 3);
	textlabel.setColourAndText(Colour::FromRGBA(colour), text);
	return true;
}

SCRIPT_API(IsValid3DTextLabel, bool(ITextLabel* textLabel))
{
	return textLabel != nullptr;
}

SCRIPT_API(Is3DTextLabelStreamedIn, bool(IPlayer& player, ITextLabel& textLabel))
{
	return textLabel.isStreamedInForPlayer(player);
}

SCRIPT_API(Get3DTextLabelText, bool(ITextLabel& textLabel, OutputOnlyString& output))
{
	output = textLabel.getText();
	return true;
}

SCRIPT_API(Get3DTextLabelColor, uint32_t(ITextLabel& textLabel))
{
	return textLabel.getColour().RGBA();
}

SCRIPT_API(Get3DTextLabelPos, bool(ITextLabel& textLabel, Vector3& out))
{
	out = textLabel.getPosition();
	return true;
}

SCRIPT_API(Set3DTextLabelDrawDistance, bool(ITextLabel& textLabel, float distance))
{
	textLabel.setDrawDistance(distance);
	return true;
}

SCRIPT_API(Get3DTextLabelDrawDistance, float(ITextLabel& textLabel))
{
	return textLabel.getDrawDistance();
}

SCRIPT_API(Get3DTextLabelLOS, bool(ITextLabel& textLabel))
{
	return textLabel.getTestLOS();
}

SCRIPT_API(Set3DTextLabelLOS, bool(ITextLabel& textLabel, bool status))
{
	textLabel.setTestLOS(status);
	return true;
}

SCRIPT_API(Get3DTextLabelVirtualWorld, int(ITextLabel& textLabel))
{
	return textLabel.getVirtualWorld();
}

SCRIPT_API(Set3DTextLabelVirtualWorld, int(ITextLabel& textLabel, int world))
{
	textLabel.setVirtualWorld(world);
	return true;
}

SCRIPT_API(Get3DTextLabelAttachedData, int(ITextLabel& textLabel, int& attached_player, int& attached_vehicle))
{
	const TextLabelAttachmentData& data = textLabel.getAttachmentData();

	attached_player = data.playerID;
	attached_vehicle = data.vehicleID;

	return true;
}

SCRIPT_API_FAILRET(CreatePlayer3DTextLabel, INVALID_TEXT_LABEL_ID, int(IPlayer& player, cell const* format, uint32_t colour, Vector3 position, float drawDistance, IPlayer* attachedPlayer, IVehicle* attachedVehicle, bool los))
{
	IPlayerTextLabelData* labelData = queryExtension<IPlayerTextLabelData>(player);
	if (labelData)
	{
		IPlayerTextLabel* textlabel = nullptr;

		AmxStringFormatter text(format, GetAMX(), GetParams(), 10); // Not 8
		if (attachedPlayer)
		{
			textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los, *attachedPlayer);
		}
		else if (attachedVehicle)
		{
			textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los, *attachedVehicle);
		}
		else
		{
			textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los);
		}

		if (textlabel)
		{
			return textlabel->getID();
		}
	}
	return FailRet;
}

SCRIPT_API(DeletePlayer3DTextLabel, bool(IPlayer& player, IPlayerTextLabel& textlabel))
{
	auto data = queryExtension<IPlayerTextLabelData>(player);
	if (!data)
	{
		return false;
	}
	data->release(textlabel.getID());
	return true;
}

SCRIPT_API(UpdatePlayer3DTextLabelText, bool(IPlayer& player, IPlayerTextLabel& textlabel, uint32_t colour, cell const* format))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 4);
	textlabel.setColourAndText(Colour::FromRGBA(colour), text);
	return true;
}

SCRIPT_API(IsValidPlayer3DTextLabel, bool(IPlayer& player, IPlayerTextLabel* textLabel))
{
	return textLabel != nullptr;
}

SCRIPT_API(GetPlayer3DTextLabelText, bool(IPlayer& player, IPlayerTextLabel& textLabel, OutputOnlyString& output))
{
	output = textLabel.getText();
	return true;
}

SCRIPT_API(GetPlayer3DTextLabelColor, uint32_t(IPlayer& player, IPlayerTextLabel& textLabel))
{
	return textLabel.getColour().RGBA();
}

SCRIPT_API(GetPlayer3DTextLabelPos, bool(IPlayer& player, IPlayerTextLabel& textLabel, Vector3& out))
{
	out = textLabel.getPosition();
	return true;
}

SCRIPT_API(SetPlayer3DTextLabelDrawDist, bool(IPlayer& player, IPlayerTextLabel& textLabel, float distance))
{
	textLabel.setDrawDistance(distance);
	return true;
}

SCRIPT_API(SetPlayer3DTextLabelDrawDistance, bool(IPlayer& player, IPlayerTextLabel& textLabel, float distance))
{
	textLabel.setDrawDistance(distance);
	return true;
}

SCRIPT_API(GetPlayer3DTextLabelDrawDist, float(IPlayer& player, IPlayerTextLabel& textLabel))
{
	return textLabel.getDrawDistance();
}

SCRIPT_API(GetPlayer3DTextLabelDrawDistance, float(IPlayer& player, IPlayerTextLabel& textLabel))
{
	return textLabel.getDrawDistance();
}

SCRIPT_API(GetPlayer3DTextLabelLOS, bool(IPlayer& player, IPlayerTextLabel& textLabel))
{
	return textLabel.getTestLOS();
}

SCRIPT_API(SetPlayer3DTextLabelLOS, bool(IPlayer& player, IPlayerTextLabel& textLabel, bool status))
{
	textLabel.setTestLOS(status);
	return true;
}

SCRIPT_API(GetPlayer3DTextLabelVirtualW, int(IPlayer& player, IPlayerTextLabel& textLabel))
{
	// No virtual world support, returning player virtual world.
	return player.getVirtualWorld();
}

SCRIPT_API(GetPlayer3DTextLabelVirtualWorld, int(IPlayer& player, IPlayerTextLabel& textLabel))
{
	// No virtual world support, returning player virtual world.
	return player.getVirtualWorld();
}

//  SCRIPT_API(SetPlayer3DTextLabelVirtualWorld, int(IPlayer& player, IPlayerTextLabel& textLabel))
//  {
//      Why is there no virtual world support?
//      return player.setVirtualWorld();
//  }

SCRIPT_API(GetPlayer3DTextLabelAttached, int(IPlayer& player, IPlayerTextLabel& textLabel, int& attached_player, int& attached_vehicle))
{
	const TextLabelAttachmentData& data = textLabel.getAttachmentData();

	attached_player = data.playerID;
	attached_vehicle = data.vehicleID;

	return true;
}
SCRIPT_API(GetPlayer3DTextLabelAttachedData, int(IPlayer& player, IPlayerTextLabel& textLabel, int& attached_player, int& attached_vehicle))
{
	const TextLabelAttachmentData& data = textLabel.getAttachmentData();

	attached_player = data.playerID;
	attached_vehicle = data.vehicleID;

	return true;
}
