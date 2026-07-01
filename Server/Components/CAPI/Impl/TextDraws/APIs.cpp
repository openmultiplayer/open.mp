/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(TextDraw_Create, objectPtr(float x, float y, StringCharPtr text, int* id))
{
	ITextDrawsComponent* component = ComponentManager::Get()->textdraws;
	if (component)
	{
		ITextDraw* textdraw = component->create({ x, y }, text);
		if (textdraw)
		{
			*id = textdraw->getID();
			return textdraw;
		}
	}
	return nullptr;
}

OMP_CAPI(TextDraw_Destroy, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	ComponentManager::Get()->textdraws->release(textdraw_->getID());
	return true;
}

OMP_CAPI(TextDraw_FromID, objectPtr(int textdrawid))
{
	ITextDrawsComponent* component = ComponentManager::Get()->textdraws;
	if (component)
	{
		return component->get(textdrawid);
	}
	return nullptr;
}

OMP_CAPI(TextDraw_GetID, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, INVALID_TEXTDRAW);
	return textdraw_->getID();
}

OMP_CAPI(TextDraw_IsValid, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	if (!textdraws->get(textdraw_->getID()))
		return false;
	return true;
}

OMP_CAPI(TextDraw_IsVisibleForPlayer, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	return textdraw_->isShownForPlayer(*player_);
}

OMP_CAPI(TextDraw_SetLetterSize, bool(objectPtr textdraw, float sizeX, float sizeY))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setLetterSize({ sizeX, sizeY });
	return true;
}

OMP_CAPI(TextDraw_SetTextSize, bool(objectPtr textdraw, float sizeX, float sizeY))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setTextSize({ sizeX, sizeY });
	return true;
}

OMP_CAPI(TextDraw_SetAlignment, bool(objectPtr textdraw, int alignment))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setAlignment(TextDrawAlignmentTypes(alignment));
	return true;
}

OMP_CAPI(TextDraw_SetColor, bool(objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(TextDraw_SetUseBox, bool(objectPtr textdraw, bool use))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->useBox(use);
	return true;
}

OMP_CAPI(TextDraw_SetBoxColor, bool(objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setBoxColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(TextDraw_SetShadow, bool(objectPtr textdraw, int size))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setShadow(size);
	return true;
}

OMP_CAPI(TextDraw_SetOutline, bool(objectPtr textdraw, int size))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setOutline(size);
	return true;
}

OMP_CAPI(TextDraw_SetBackgroundColor, bool(objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setBackgroundColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(TextDraw_SetFont, bool(objectPtr textdraw, int font))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setStyle(TextDrawStyle(font));
	return true;
}

OMP_CAPI(TextDraw_SetProportional, bool(objectPtr textdraw, bool set))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setProportional(set);
	return true;
}

OMP_CAPI(TextDraw_SetSelectable, bool(objectPtr textdraw, bool set))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setSelectable(set);
	return true;
}

OMP_CAPI(TextDraw_ShowForPlayer, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->showForPlayer(*player_);
	return true;
}

OMP_CAPI(TextDraw_HideForPlayer, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->hideForPlayer(*player_);
	return true;
}

OMP_CAPI(TextDraw_ShowForAll, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	IPlayerPool* pool = ComponentManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		textdraw_->showForPlayer(*player);
	}
	return true;
}

OMP_CAPI(TextDraw_HideForAll, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	IPlayerPool* pool = ComponentManager::Get()->players;
	for (IPlayer* player : pool->entries())
	{
		textdraw_->hideForPlayer(*player);
	}
	return true;
}

OMP_CAPI(TextDraw_SetString, bool(objectPtr textdraw, StringCharPtr text))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setText(text);
	return true;
}

OMP_CAPI(TextDraw_SetPreviewModel, bool(objectPtr textdraw, int model))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setPreviewModel(model);
	return true;
}

OMP_CAPI(TextDraw_SetPreviewRot, bool(objectPtr textdraw, float rotationX, float rotationY, float rotationZ, float zoom))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setPreviewRotation({ rotationX, rotationY, rotationZ });
	textdraw_->setPreviewZoom(zoom);
	return true;
}

OMP_CAPI(TextDraw_SetPreviewVehCol, bool(objectPtr textdraw, int color1, int color2))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setPreviewVehicleColour(color1, color2);
	return true;
}

OMP_CAPI(TextDraw_SetPos, bool(objectPtr textdraw, float x, float y))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	textdraw_->setPosition({ x, y });
	return true;
}

OMP_CAPI(TextDraw_GetString, bool(objectPtr textdraw, OutputStringViewPtr text))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	auto textStr = textdraw_->getText();
	SET_CAPI_STRING_VIEW(text, textStr);
	return true;
}

OMP_CAPI(TextDraw_GetLetterSize, bool(objectPtr textdraw, float* sizeX, float* sizeY))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	const Vector2& size = textdraw_->getLetterSize();
	*sizeX = size.x;
	*sizeY = size.y;
	return true;
}

OMP_CAPI(TextDraw_GetTextSize, bool(objectPtr textdraw, float* sizeX, float* sizeY))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	const Vector2& size = textdraw_->getTextSize();
	*sizeX = size.x;
	*sizeY = size.y;
	return true;
}

OMP_CAPI(TextDraw_GetPos, bool(objectPtr textdraw, float* x, float* y))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	const Vector2& pos = textdraw_->getPosition();
	*x = pos.x;
	*y = pos.y;
	return true;
}

OMP_CAPI(TextDraw_GetColor, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getLetterColour().RGBA();
}

OMP_CAPI(TextDraw_GetBoxColor, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getBoxColour().RGBA();
}

OMP_CAPI(TextDraw_GetBackgroundColor, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getBackgroundColour().RGBA();
}

OMP_CAPI(TextDraw_GetShadow, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getShadow();
}

OMP_CAPI(TextDraw_GetOutline, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getOutline();
}

OMP_CAPI(TextDraw_GetFont, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, -1);
	return static_cast<int>(textdraw_->getStyle());
}

OMP_CAPI(TextDraw_IsBox, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	return textdraw_->hasBox();
}

OMP_CAPI(TextDraw_IsProportional, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	return textdraw_->isProportional();
}

OMP_CAPI(TextDraw_IsSelectable, bool(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	return textdraw_->isSelectable();
}

OMP_CAPI(TextDraw_GetAlignment, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return static_cast<int>(textdraw_->getAlignment());
}

OMP_CAPI(TextDraw_GetPreviewModel, int(objectPtr textdraw))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, 0);
	return textdraw_->getPreviewModel();
}

OMP_CAPI(TextDraw_GetPreviewRot, bool(objectPtr textdraw, float* x, float* y, float* z, float* zoom))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	const Vector3& rotation = textdraw_->getPreviewRotation();
	*x = rotation.x;
	*y = rotation.y;
	*z = rotation.z;
	*zoom = textdraw_->getPreviewZoom();
	return true;
}

OMP_CAPI(TextDraw_GetPreviewVehColor, bool(objectPtr textdraw, int* color1, int* color2))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	Pair<int, int> colors = textdraw_->getPreviewVehicleColour();
	*color1 = colors.first;
	*color2 = colors.second;
	return true;
}

OMP_CAPI(TextDraw_SetStringForPlayer, bool(objectPtr textdraw, objectPtr player, StringCharPtr text))
{
	POOL_ENTITY_RET(textdraws, ITextDraw, textdraw, textdraw_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	textdraw_->setTextForPlayer(*player_, text);
	return true;
}

/*
	Per-Player TextDraws
*/

OMP_CAPI(PlayerTextDraw_Create, objectPtr(objectPtr player, float x, float y, StringCharPtr text, int* id))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerTextDrawData* playerTextDraws = GetPlayerData<IPlayerTextDrawData>(player_);
	if (playerTextDraws)
	{
		IPlayerTextDraw* textdraw = playerTextDraws->create({ x, y }, text);
		if (textdraw)
		{
			*id = textdraw->getID();
			return textdraw;
		}
	}
	return nullptr;
}

OMP_CAPI(PlayerTextDraw_Destroy, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	if (playerData)
	{
		playerData->release(td->getID());
		return true;
	}
	return false;
}

OMP_CAPI(PlayerTextDraw_FromID, objectPtr(objectPtr player, int textdrawid))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerTextDrawData* playerTextDraws = GetPlayerData<IPlayerTextDrawData>(player_);
	if (playerTextDraws)
	{
		return playerTextDraws->get(textdrawid);
	}
	return nullptr;
}

OMP_CAPI(PlayerTextDraw_GetID, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, INVALID_TEXTDRAW);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, textdraw_, INVALID_TEXTDRAW);
	return textdraw_->getID();
}

OMP_CAPI(PlayerTextDraw_IsValid, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	if (!playerData->get(td->getID()))
		return false;
	return true;
}

OMP_CAPI(PlayerTextDraw_IsVisible, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	return td->isShown();
}

OMP_CAPI(PlayerTextDraw_SetLetterSize, bool(objectPtr player, objectPtr textdraw, float x, float y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setLetterSize({ x, y });
	return true;
}

OMP_CAPI(PlayerTextDraw_SetTextSize, bool(objectPtr player, objectPtr textdraw, float x, float y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setTextSize({ x, y });
	return true;
}

OMP_CAPI(PlayerTextDraw_SetAlignment, bool(objectPtr player, objectPtr textdraw, int alignment))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setAlignment(TextDrawAlignmentTypes(alignment));
	return true;
}

OMP_CAPI(PlayerTextDraw_SetColor, bool(objectPtr player, objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(PlayerTextDraw_UseBox, bool(objectPtr player, objectPtr textdraw, bool use))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->useBox(use);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetBoxColor, bool(objectPtr player, objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setBoxColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(PlayerTextDraw_SetShadow, bool(objectPtr player, objectPtr textdraw, int size))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setShadow(size);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetOutline, bool(objectPtr player, objectPtr textdraw, int size))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setOutline(size);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetBackgroundColor, bool(objectPtr player, objectPtr textdraw, uint32_t color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setBackgroundColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(PlayerTextDraw_SetFont, bool(objectPtr player, objectPtr textdraw, int font))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setStyle(TextDrawStyle(font));
	return true;
}

OMP_CAPI(PlayerTextDraw_SetProportional, bool(objectPtr player, objectPtr textdraw, bool set))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setProportional(set);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetSelectable, bool(objectPtr player, objectPtr textdraw, bool set))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setSelectable(set);
	return true;
}

OMP_CAPI(PlayerTextDraw_Show, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->show();
	return true;
}

OMP_CAPI(PlayerTextDraw_Hide, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->hide();
	return true;
}

OMP_CAPI(PlayerTextDraw_SetString, bool(objectPtr player, objectPtr textdraw, StringCharPtr text))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setText(text);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetPreviewModel, bool(objectPtr player, objectPtr textdraw, int model))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setPreviewModel(model);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetPreviewRot, bool(objectPtr player, objectPtr textdraw, float rx, float ry, float rz, float zoom))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setPreviewRotation({ rx, ry, rz });
	td->setPreviewZoom(zoom);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetPreviewVehCol, bool(objectPtr player, objectPtr textdraw, int color1, int color2))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setPreviewVehicleColour(color1, color2);
	return true;
}

OMP_CAPI(PlayerTextDraw_SetPos, bool(objectPtr player, objectPtr textdraw, float x, float y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	td->setPosition({ x, y });
	return true;
}

OMP_CAPI(PlayerTextDraw_GetString, bool(objectPtr player, objectPtr textdraw, OutputStringViewPtr text))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	auto result = td->getText();
	SET_CAPI_STRING_VIEW(text, result);
	return true;
}

OMP_CAPI(PlayerTextDraw_GetLetterSize, bool(objectPtr player, objectPtr textdraw, float* x, float* y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	auto size = td->getLetterSize();
	*x = size.x;
	*y = size.y;
	return true;
}

OMP_CAPI(PlayerTextDraw_GetTextSize, bool(objectPtr player, objectPtr textdraw, float* x, float* y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	auto size = td->getTextSize();
	*x = size.x;
	*y = size.y;
	return true;
}

OMP_CAPI(PlayerTextDraw_GetPos, bool(objectPtr player, objectPtr textdraw, float* x, float* y))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	auto pos = td->getPosition();
	*x = pos.x;
	*y = pos.y;
	return true;
}

OMP_CAPI(PlayerTextDraw_GetColor, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getLetterColour().RGBA();
}

OMP_CAPI(PlayerTextDraw_GetBoxColor, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getBoxColour().RGBA();
}

OMP_CAPI(PlayerTextDraw_GetBackgroundColor, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getBackgroundColour().RGBA();
}

OMP_CAPI(PlayerTextDraw_GetShadow, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getShadow();
}

OMP_CAPI(PlayerTextDraw_GetOutline, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getOutline();
}

OMP_CAPI(PlayerTextDraw_GetFont, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, -1);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, -1);
	return td->getStyle();
}

OMP_CAPI(PlayerTextDraw_IsBox, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	return td->hasBox();
}

OMP_CAPI(PlayerTextDraw_IsProportional, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	return td->isProportional();
}

OMP_CAPI(PlayerTextDraw_IsSelectable, bool(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	return td->isSelectable();
}

OMP_CAPI(PlayerTextDraw_GetAlignment, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return static_cast<uint8_t>(td->getAlignment());
}

OMP_CAPI(PlayerTextDraw_GetPreviewModel, int(objectPtr player, objectPtr textdraw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, 0);
	return td->getPreviewModel();
}

OMP_CAPI(PlayerTextDraw_GetPreviewRot, bool(objectPtr player, objectPtr textdraw, float* rx, float* ry, float* rz, float* zoom))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	auto rotation = td->getPreviewRotation();

	*rx = rotation.x;
	*ry = rotation.y;
	*rz = rotation.z;
	*zoom = td->getPreviewZoom();
	return true;
}

OMP_CAPI(PlayerTextDraw_GetPreviewVehColor, bool(objectPtr player, objectPtr textdraw, int* color1, int* color2))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerTextDrawData, IPlayerTextDraw, textdraw, td, false);
	Pair<int, int> colours = td->getPreviewVehicleColour();
	*color1 = colours.first;
	*color2 = colours.second;
	return true;
}
