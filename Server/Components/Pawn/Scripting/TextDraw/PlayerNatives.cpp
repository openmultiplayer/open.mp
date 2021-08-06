#include "sdk.hpp"
#include <iostream>
#include "Server/Components/TextDraws/textdraws.hpp"
#include "../Types.hpp"

SCRIPT_API(CreatePlayerTextDraw, int(IPlayer& player, Vector2 position, const std::string& text))
{
    IPlayerTextDrawData* playerTextDraws = player.queryData<IPlayerTextDrawData>();
    if (playerTextDraws) {
        IPlayerTextDraw* textdraw = playerTextDraws->create(position, text);
        if (textdraw) {
            return textdraw->getID();
        }
    }
    return INVALID_TEXTDRAW;
}

SCRIPT_API(PlayerTextDrawDestroy, bool(IPlayer& player, IPlayerTextDraw& textdraw))
{
    IPlayerTextDrawData* playerTextDraws = player.queryData<IPlayerTextDrawData>();
    if (playerTextDraws) {
        playerTextDraws->release(textdraw.getID());
        return true;
    }
    return false;
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
    textdraw.setLetterColour(Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(PlayerTextDrawUseBox, bool(IPlayer& player, IPlayerTextDraw& textdraw, bool use))
{
    textdraw.setUsingBox(use);
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
    textdraw.setBackColour(Colour::FromRGBA(colour));
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

SCRIPT_API(PlayerTextDrawSetString, bool(IPlayer& player, IPlayerTextDraw& textdraw, const std::string& text))
{
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
    textdraw.setPreviewRotation(GTAQuat(rotation));
    textdraw.setPreviewZoom(zoom);
    return true;
}

SCRIPT_API(PlayerTextDrawSetPreviewVehCol, bool(IPlayer& player, IPlayerTextDraw& textdraw, int colour1, int colour2))
{
    textdraw.setPreviewVehicleColour(colour1, colour2);
    return true;
}
