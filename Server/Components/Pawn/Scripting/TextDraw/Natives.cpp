#include "sdk.hpp"
#include <iostream>
#include "Server/Components/TextDraws/textdraws.hpp"
#include "../Types.hpp"

SCRIPT_API(TextDrawCreate, int(Vector2 position, const std::string& text))
{
    ITextDrawsComponent* component = PawnManager::Get()->textdraws;
    if (component) {
        ITextDraw* textdraw = component->create(position, text);
        if (textdraw) {
            return textdraw->getID();
        }
    }
    return INVALID_TEXTDRAW;
}

SCRIPT_API(TextDrawDestroy, bool(ITextDraw& textdraw))
{
    PawnManager::Get()->textdraws->release(textdraw.getID());
    return true;
}

SCRIPT_API(TextDrawLetterSize, bool(ITextDraw& textdraw, Vector2 size))
{
    textdraw.setLetterSize(size);
    return true;
}

SCRIPT_API(TextDrawTextSize, bool(ITextDraw& textdraw, Vector2 size))
{
    textdraw.setTextSize(size);
    return true;
}

SCRIPT_API(TextDrawAlignment, bool(ITextDraw& textdraw, int alignment))
{
    textdraw.setAlignment(TextDrawAlignmentTypes(alignment));
    return true;
}

SCRIPT_API(TextDrawColor, bool(ITextDraw& textdraw, uint32_t colour))
{
    textdraw.setLetterColour(Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(TextDrawUseBox, bool(ITextDraw& textdraw, bool use))
{
    textdraw.setUsingBox(use);
    return true;
}

SCRIPT_API(TextDrawBoxColor, bool(ITextDraw& textdraw, uint32_t colour))
{
    textdraw.setBoxColour(Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(TextDrawSetShadow, bool(ITextDraw& textdraw, int size))
{
    textdraw.setShadow(size);
    return true;
}

SCRIPT_API(TextDrawSetOutline, bool(ITextDraw& textdraw, int size))
{
    textdraw.setOutline(size);
    return true;
}

SCRIPT_API(TextDrawBackgroundColor, bool(ITextDraw& textdraw, uint32_t colour))
{
    textdraw.setBackColour(Colour::FromRGBA(colour));
    return true;
}

SCRIPT_API(TextDrawFont, bool(ITextDraw& textdraw, int font))
{
    textdraw.setStyle(TextDrawStyle(font));
    return true;
}

SCRIPT_API(TextDrawSetProportional, bool(ITextDraw& textdraw, bool set))
{
    textdraw.setProportional(set);
    return true;
}

SCRIPT_API(TextDrawSetSelectable, bool(ITextDraw& textdraw, bool set))
{
    textdraw.setSelectable(set);
    return true;
}

SCRIPT_API(TextDrawShowForPlayer, bool(IPlayer& player, ITextDraw& textdraw))
{
    textdraw.showForPlayer(player);
    return true;
}

SCRIPT_API(TextDrawHideForPlayer, bool(IPlayer& player, ITextDraw& textdraw))
{
    textdraw.hideForPlayer(player);
    return true;
}

SCRIPT_API(TextDrawShowForAll, bool(ITextDraw& textdraw))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        textdraw.showForPlayer(*player);
    }
    return true;
}

SCRIPT_API(TextDrawHideForAll, bool(ITextDraw& textdraw))
{
    IPlayerPool* pool = PawnManager::Get()->players;
    for (IPlayer* player : pool->entries()) {
        textdraw.hideForPlayer(*player);
    }
    return true;
}

SCRIPT_API(TextDrawSetString, bool(ITextDraw& textdraw, const std::string& text))
{
    textdraw.setText(text);
    return true;
}

SCRIPT_API(TextDrawSetPreviewModel, bool(ITextDraw& textdraw, int model))
{
    textdraw.setPreviewModel(model);
    return true;
}

SCRIPT_API(TextDrawSetPreviewRot, bool(ITextDraw& textdraw, Vector3 rotation, float zoom))
{
    textdraw.setPreviewRotation(GTAQuat(rotation));
    textdraw.setPreviewZoom(zoom);
    return true;
}

SCRIPT_API(TextDrawSetPreviewVehCol, bool(ITextDraw& textdraw, int colour1, int colour2))
{
    textdraw.setPreviewVehicleColour(colour1, colour2);
    return true;
}
