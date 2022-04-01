#include "../Types.hpp"
#include "Server/Components/TextDraws/textdraws.hpp"
#include "sdk.hpp"
#include <iostream>

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

SCRIPT_API(IsValidTextDraw, bool(ITextDraw* textdraw))
{
    return textdraw != nullptr;
}

SCRIPT_API(IsTextDrawVisibleForPlayer, bool(IPlayer& player, ITextDraw& textdraw))
{
    return textdraw.isShownForPlayer(player);
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

SCRIPT_API(TextDrawSetPos, bool(ITextDraw& textdraw, Vector2 pos))
{
    textdraw.setPosition(pos);
    return true;
}

SCRIPT_API(TextDrawGetString, bool(ITextDraw& textdraw, OutputOnlyString& text))
{
    text = textdraw.getText();
    return true;
}

SCRIPT_API(TextDrawGetLetterSize, bool(ITextDraw& textdraw, Vector2& size))
{
    size = textdraw.getLetterSize();
    return true;
}

SCRIPT_API(TextDrawGetTextSize, bool(ITextDraw& textdraw, Vector2& size))
{
    size = textdraw.getTextSize();
    return true;
}

SCRIPT_API(TextDrawGetPos, bool(ITextDraw& textdraw, Vector2& pos))
{
    pos = textdraw.getPosition();
    return true;
}

SCRIPT_API(TextDrawGetColor, int(ITextDraw& textdraw))
{
    return textdraw.getLetterColour().RGBA();
}

SCRIPT_API(TextDrawGetBoxColor, int(ITextDraw& textdraw))
{
    return textdraw.getBoxColour().RGBA();
}

SCRIPT_API(TextDrawGetBackgroundColor, int(ITextDraw& textdraw))
{
    return textdraw.getBackColour().RGBA();
}

SCRIPT_API(TextDrawGetShadow, int(ITextDraw& textdraw))
{
    return textdraw.getShadow();
}

SCRIPT_API(TextDrawGetOutline, int(ITextDraw& textdraw))
{
    return textdraw.getOutline();
}

SCRIPT_API(TextDrawGetFont, int(ITextDraw& textdraw))
{
    return static_cast<uint8_t>(textdraw.getStyle());
}

SCRIPT_API(TextDrawIsBox, int(ITextDraw& textdraw))
{
    return textdraw.isUsingBox();
}

SCRIPT_API(TextDrawIsProportional, int(ITextDraw& textdraw))
{
    return textdraw.isProportional();
}

SCRIPT_API(TextDrawIsSelectable, int(ITextDraw& textdraw))
{
    return textdraw.isSelectable();
}

SCRIPT_API(TextDrawGetAlignment, int(ITextDraw& textdraw))
{
    return static_cast<uint8_t>(textdraw.getAlignment());
}

SCRIPT_API(TextDrawGetPreviewModel, int(ITextDraw& textdraw))
{
    return textdraw.getPreviewModel();
}

SCRIPT_API(TextDrawGetPreviewRot, bool(ITextDraw& textdraw, Vector3& rotation, float& zoom))
{
    rotation = textdraw.getPreviewRotation().ToEuler();
    zoom = textdraw.getPreviewZoom();
    return true;
}

SCRIPT_API(TextDrawGetPreviewVehCol, bool(ITextDraw& textdraw, int& colour1, int& colour2))
{
    Pair<int, int> colours = textdraw.getPreviewVehicleColour();
    colour1 = colours.first;
    colour2 = colours.second;
    return true;
}
