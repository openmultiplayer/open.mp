/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "Server/Components/TextDraws/textdraws.hpp"
#include "sdk.hpp"
#include <iostream>
#include "../../format.hpp"

SCRIPT_API(CreatePlayerTextDraw, int(IPlayer& player, Vector2 position, cell const* format))
{
	IPlayerTextDrawData* playerTextDraws = queryExtension<IPlayerTextDrawData>(player);
	if (playerTextDraws)
	{
		AmxStringFormatter text(format, GetAMX(), GetParams(), 4); // Not 3
		IPlayerTextDraw* textdraw = playerTextDraws->create(position, text);
		if (textdraw)
		{
			return textdraw->getID();
		}
	}
	return INVALID_TEXTDRAW;
}

SCRIPT_API(PlayerTextDrawDestroy, bool(IPlayer& player, IPlayerTextDraw& textdraw))
{
	IPlayerTextDrawData* playerTextDraws = queryExtension<IPlayerTextDrawData>(player);
	if (playerTextDraws)
	{
		playerTextDraws->release(textdraw.getID());
		return true;
	}
	return false;
}

SCRIPT_API(IsValidPlayerTextDraw, bool(IPlayer& player, IPlayerTextDraw* textdraw))
{
	return textdraw != nullptr;
}

SCRIPT_API(IsPlayerTextDrawVisible, bool(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.isShown();
}

SCRIPT_API(PlayerTextDrawLetterSize, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2 size))
{
	textdraw.setLetterSize(size);
	return true;
}

SCRIPT_API(PlayerTextDrawTextSize, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2 size))
{
	textdraw.setTextSize(size);
	return true;
}

SCRIPT_API(PlayerTextDrawAlignment, bool(IPlayer& player, IPlayerTextDraw& textdraw, int alignment))
{
	textdraw.setAlignment(TextDrawAlignmentTypes(alignment));
	return true;
}

SCRIPT_API(PlayerTextDrawColor, bool(IPlayer& player, IPlayerTextDraw& textdraw, uint32_t colour))
{
	textdraw.setColour(Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerTextDrawUseBox, bool(IPlayer& player, IPlayerTextDraw& textdraw, bool use))
{
	textdraw.useBox(use);
	return true;
}

SCRIPT_API(PlayerTextDrawBoxColor, bool(IPlayer& player, IPlayerTextDraw& textdraw, uint32_t colour))
{
	textdraw.setBoxColour(Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerTextDrawSetShadow, bool(IPlayer& player, IPlayerTextDraw& textdraw, int size))
{
	textdraw.setShadow(size);
	return true;
}

SCRIPT_API(PlayerTextDrawSetOutline, bool(IPlayer& player, IPlayerTextDraw& textdraw, int size))
{
	textdraw.setOutline(size);
	return true;
}

SCRIPT_API(PlayerTextDrawBackgroundColor, bool(IPlayer& player, IPlayerTextDraw& textdraw, uint32_t colour))
{
	textdraw.setBackgroundColour(Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(PlayerTextDrawFont, bool(IPlayer& player, IPlayerTextDraw& textdraw, int font))
{
	textdraw.setStyle(TextDrawStyle(font));
	return true;
}

SCRIPT_API(PlayerTextDrawSetProportional, bool(IPlayer& player, IPlayerTextDraw& textdraw, bool set))
{
	textdraw.setProportional(set);
	return true;
}

SCRIPT_API(PlayerTextDrawSetSelectable, bool(IPlayer& player, IPlayerTextDraw& textdraw, bool set))
{
	textdraw.setSelectable(set);
	return true;
}

SCRIPT_API(PlayerTextDrawShow, bool(IPlayer& player, IPlayerTextDraw& textdraw))
{
	textdraw.show();
	return true;
}

SCRIPT_API(PlayerTextDrawHide, bool(IPlayer& player, IPlayerTextDraw& textdraw))
{
	textdraw.hide();
	return true;
}

SCRIPT_API(PlayerTextDrawSetString, bool(IPlayer& player, IPlayerTextDraw& textdraw, cell const* format))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 3);
	textdraw.setText(text);
	return true;
}

SCRIPT_API(PlayerTextDrawSetPreviewModel, bool(IPlayer& player, IPlayerTextDraw& textdraw, int model))
{
	textdraw.setPreviewModel(model);
	return true;
}

SCRIPT_API(PlayerTextDrawSetPreviewRot, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector3 rotation, float zoom))
{
	textdraw.setPreviewRotation(rotation);
	textdraw.setPreviewZoom(zoom);
	return true;
}

SCRIPT_API(PlayerTextDrawSetPreviewVehCol, bool(IPlayer& player, IPlayerTextDraw& textdraw, int colour1, int colour2))
{
	textdraw.setPreviewVehicleColour(colour1, colour2);
	return true;
}

SCRIPT_API(PlayerTextDrawSetPos, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2 pos))
{
	textdraw.setPosition(pos);
	return true;
}

SCRIPT_API(PlayerTextDrawGetString, bool(IPlayer& player, IPlayerTextDraw& textdraw, OutputOnlyString& text))
{
	text = textdraw.getText();
	return true;
}

SCRIPT_API(PlayerTextDrawGetLetterSize, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2& size))
{
	size = textdraw.getLetterSize();
	return true;
}

SCRIPT_API(PlayerTextDrawGetTextSize, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2& size))
{
	size = textdraw.getTextSize();
	return true;
}

SCRIPT_API(PlayerTextDrawGetPos, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector2& pos))
{
	pos = textdraw.getPosition();
	return true;
}

SCRIPT_API(PlayerTextDrawGetColor, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getLetterColour().RGBA();
}

SCRIPT_API(PlayerTextDrawGetBoxColor, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getBoxColour().RGBA();
}

SCRIPT_API(PlayerTextDrawGetBackgroundCol, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getBackgroundColour().RGBA();
}

SCRIPT_API(PlayerTextDrawGetShadow, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getShadow();
}

SCRIPT_API(PlayerTextDrawGetOutline, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getOutline();
}

SCRIPT_API(PlayerTextDrawGetFont, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return static_cast<uint8_t>(textdraw.getStyle());
}

SCRIPT_API(PlayerTextDrawIsBox, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.hasBox();
}

SCRIPT_API(PlayerTextDrawIsProportional, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.isProportional();
}

SCRIPT_API(PlayerTextDrawIsSelectable, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.isSelectable();
}

SCRIPT_API(PlayerTextDrawGetAlignment, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return static_cast<uint8_t>(textdraw.getAlignment());
}

SCRIPT_API(PlayerTextDrawGetPreviewModel, int(IPlayer& player, IPlayerTextDraw& textdraw))
{
	return textdraw.getPreviewModel();
}

SCRIPT_API(PlayerTextDrawGetPreviewRot, bool(IPlayer& player, IPlayerTextDraw& textdraw, Vector3& rotation, float& zoom))
{
	rotation = textdraw.getPreviewRotation();
	zoom = textdraw.getPreviewZoom();
	return true;
}

SCRIPT_API(PlayerTextDrawGetPreviewVehCol, bool(IPlayer& player, IPlayerTextDraw& textdraw, int& colour1, int& colour2))
{
	Pair<int, int> colours = textdraw.getPreviewVehicleColour();
	colour1 = colours.first;
	colour2 = colours.second;
	return true;
}
