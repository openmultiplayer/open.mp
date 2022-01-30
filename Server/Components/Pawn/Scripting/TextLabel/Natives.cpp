#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API_FAILRET(Create3DTextLabel, INVALID_TEXT_LABEL_ID, int(const std::string& text, uint32_t colour, Vector3 position, float drawDistance, int virtualWorld, bool los))
{
    ITextLabelsComponent* component = PawnManager::Get()->textlabels;
    if (component) {
        ITextLabel* textlabel = component->create(text, Colour::FromRGBA(colour), position, drawDistance, virtualWorld, los);
        if (textlabel) {
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

SCRIPT_API(Update3DTextLabelText, bool(ITextLabel& textlabel, uint32_t colour, const std::string& text))
{
    textlabel.setColour(Colour::FromRGBA(colour));
    textlabel.setText(text);
    return true;
}

SCRIPT_API_FAILRET(CreatePlayer3DTextLabel, INVALID_TEXT_LABEL_ID, int(IPlayer& player, const std::string& text, uint32_t colour, Vector3 position, float drawDistance, IPlayer* attachedPlayer, IVehicle* attachedVehicle, bool los))
{
    IPlayerTextLabelData* labelData = queryData<IPlayerTextLabelData>(player);
    if (labelData) {
        IPlayerTextLabel* textlabel = nullptr;

        if (attachedPlayer) {
            textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los, *attachedPlayer);
        } else if (attachedVehicle) {
            textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los, *attachedVehicle);
        } else {
            textlabel = labelData->create(text, Colour::FromRGBA(colour), position, drawDistance, los);
        }

        if (textlabel) {
            return textlabel->getID();
        }
    }
    return FailRet;
}

SCRIPT_API(DeletePlayer3DTextLabel, bool(IPlayer& player, IPlayerTextLabel& textlabel))
{
    queryData<IPlayerTextLabelData>(player)->release(textlabel.getID());
    return true;
}

SCRIPT_API(UpdatePlayer3DTextLabelText, bool(IPlayer& player, IPlayerTextLabel& textlabel, uint32_t colour, const std::string& text))
{
    textlabel.setColour(Colour::FromRGBA(colour));
    textlabel.setText(text);
    return true;
}
