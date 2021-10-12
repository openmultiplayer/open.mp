#include <Server/Components/TextDraws/textdraws.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

template <class T>
struct TextDrawBase : public T, public PoolIDProvider, public NoCopy {
    Vector2 pos;
    String text;
    Colour letterColour = Colour(0xE1, 0xE1, 0xE1);
    Vector2 letterSize = Vector2(0.48f, 1.12f);
    Vector2 textSize = Vector2(1280.f);
    TextDrawAlignmentTypes alignment = TextDrawAlignment_Default;
    bool box = false;
    bool proportional = true;
    Colour boxColour = Colour(0x80, 0x80, 0x80, 0x80);
    int shadowSize = 2;
    int outlineSize = 0;
    Colour backgroundColour = Colour::Black();
    TextDrawStyle style = TextDrawStyle_FontAharoniBold;
    bool selectable = false;
    int previewModel = 0;
    GTAQuat previewRotation = GTAQuat(Vector3(0.f));
    Pair<int, int> previewVehicleColours = std::make_pair(-1, -1);
    float previewZoom = 1.f;

    int getID() const override
    {
        return poolID;
    }

    Vector2 getPosition() const override
    {
        return pos;
    }

    T& setPosition(Vector2 position) override
    {
        pos = position;
        return *this;
    }

    void setText(StringView txt) override
    {
        text = txt;
    }

    StringView getText() const override
    {
        return text;
    }

    T& setLetterColour(Colour col) override
    {
        letterColour = col;
        return *this;
    }

    Colour getLetterColour() const override
    {
        return letterColour;
    }

    T& setLetterSize(Vector2 size) override
    {
        letterSize = size;
        return *this;
    }

    Vector2 getLetterSize() const override
    {
        return letterSize;
    }

    T& setTextSize(Vector2 size) override
    {
        textSize = size;
        return *this;
    }

    Vector2 getTextSize() const override
    {
        return textSize;
    }

    T& setAlignment(TextDrawAlignmentTypes align) override
    {
        alignment = align;
        return *this;
    }

    TextDrawAlignmentTypes getAlignment() const override
    {
        return alignment;
    }

    T& setUsingBox(bool use) override
    {
        box = use;
        return *this;
    }

    bool isUsingBox() const override
    {
        return box;
    }

    T& setBoxColour(Colour colour) override
    {
        boxColour = colour;
        return *this;
    }

    Colour getBoxColour() const override
    {
        return boxColour;
    }

    T& setShadow(int shadow) override
    {
        shadowSize = shadow;
        return *this;
    }

    int getShadow() const override
    {
        return shadowSize;
    }

    T& setOutline(int outline) override
    {
        outlineSize = outline;
        return *this;
    }

    int getOutline() const override
    {
        return outlineSize;
    }

    T& setBackColour(Colour colour) override
    {
        backgroundColour = colour;
        return *this;
    }

    Colour getBackColour() const override
    {
        return backgroundColour;
    }

    T& setStyle(TextDrawStyle s) override
    {
        style = s;
        return *this;
    }

    TextDrawStyle getStyle() const override
    {
        return style;
    }

    T& setProportional(bool p) override
    {
        proportional = p;
        return *this;
    }

    bool isProportional() const override
    {
        return proportional;
    }

    T& setSelectable(bool select) override
    {
        selectable = select;
        return *this;
    }

    bool isSelectable() const override
    {
        return selectable;
    }

    T& setPreviewModel(int model) override
    {
        previewModel = model;
        return *this;
    }

    int getPreviewModel() const override
    {
        return previewModel;
    }

    T& setPreviewRotation(GTAQuat rotation) override
    {
        previewRotation = rotation;
        return *this;
    }

    GTAQuat getPreviewRotation() const override
    {
        return previewRotation;
    }

    T& setPreviewVehicleColour(int colour1, int colour2) override
    {
        previewVehicleColours.first = colour1;
        previewVehicleColours.second = colour2;
        return *this;
    }

    Pair<int, int> getPreviewVehicleColour() const override
    {
        return previewVehicleColours;
    }

    T& setPreviewZoom(float zoom) override
    {
        previewZoom = zoom;
        return *this;
    }

    float getPreviewZoom() const override
    {
        return previewZoom;
    }

    void showForClient(IPlayer& player, bool isPlayerTextDraw)
    {
        NetCode::RPC::PlayerShowTextDraw playerShowTextDrawRPC;
        playerShowTextDrawRPC.PlayerTextDraw = isPlayerTextDraw;
        playerShowTextDrawRPC.UseBox = box;
        switch (alignment) {
        case TextDrawAlignmentTypes::TextDrawAlignment_Default:
            playerShowTextDrawRPC.Alignment = 0;
            break;
        case TextDrawAlignmentTypes::TextDrawAlignment_Left:
            playerShowTextDrawRPC.Alignment = 1;
            break;
        case TextDrawAlignmentTypes::TextDrawAlignment_Center:
            playerShowTextDrawRPC.Alignment = 4;
            break;
        case TextDrawAlignmentTypes::TextDrawAlignment_Right:
            playerShowTextDrawRPC.Alignment = 2;
            break;
        }
        playerShowTextDrawRPC.Proportional = proportional;
        playerShowTextDrawRPC.TextDrawID = poolID;
        playerShowTextDrawRPC.LetterSize = letterSize;
        playerShowTextDrawRPC.LetterColour = letterColour;
        playerShowTextDrawRPC.TextSize = textSize;
        playerShowTextDrawRPC.BoxColour = boxColour;
        playerShowTextDrawRPC.Shadow = shadowSize;
        playerShowTextDrawRPC.Outline = outlineSize;
        playerShowTextDrawRPC.BackColour = backgroundColour;
        playerShowTextDrawRPC.Style = style;
        playerShowTextDrawRPC.Selectable = selectable;
        playerShowTextDrawRPC.Position = pos;
        playerShowTextDrawRPC.Model = previewModel;
        playerShowTextDrawRPC.Rotation = previewRotation.ToEuler();
        playerShowTextDrawRPC.Zoom = previewZoom;
        playerShowTextDrawRPC.Color1 = previewVehicleColours.first;
        playerShowTextDrawRPC.Color2 = previewVehicleColours.second;
        playerShowTextDrawRPC.Text = StringView(text);
        player.sendRPC(playerShowTextDrawRPC);
    }

    void hideForClient(IPlayer& player, bool isPlayerTextDraw)
    {
        NetCode::RPC::PlayerHideTextDraw playerHideTextDrawRPC;
        playerHideTextDrawRPC.PlayerTextDraw = isPlayerTextDraw;
        playerHideTextDrawRPC.TextDrawID = poolID;
        player.sendRPC(playerHideTextDrawRPC);
    }

    void setTextForClient(IPlayer& player, StringView txt, bool isPlayerTextDraw)
    {
        NetCode::RPC::PlayerTextDrawSetString playerTextDrawSetStringRPC;
        playerTextDrawSetStringRPC.PlayerTextDraw = isPlayerTextDraw;
        playerTextDrawSetStringRPC.TextDrawID = poolID;
        playerTextDrawSetStringRPC.Text = txt;
        player.sendRPC(playerTextDrawSetStringRPC);
    }
};

struct TextDraw final : public TextDrawBase<ITextDraw> {
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> shownFor_;

    void restream() override
    {
        for (IPlayer* player : shownFor_.entries()) {
            showForClient(*player, false);
        }
    }

    bool isShownForPlayer(const IPlayer& player) const override
    {
        return shownFor_.valid(player.getID());
    }

    void showForPlayer(IPlayer& player) override
    {
        shownFor_.add(player.getID(), player);
        showForClient(player, false);
    }

    void hideForPlayer(IPlayer& player) override
    {
        shownFor_.remove(player.getID(), player);
        hideForClient(player, false);
    }

    void setText(StringView txt) override
    {
        TextDrawBase<ITextDraw>::setText(txt);
        for (IPlayer* player : shownFor_.entries()) {
            setTextForClient(*player, txt, false);
        }
    }

    ~TextDraw()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForClient(*player, false);
        }
    }
};

struct PlayerTextDraw final : public TextDrawBase<IPlayerTextDraw> {
    IPlayer* player = nullptr;
    bool shown = false;

    void show() override
    {
        showForClient(*player, true);
        shown = true;
    }

    void hide() override
    {
        hideForClient(*player, true);
        shown = false;
    }

    bool isShown() const override
    {
        return shown;
    }

    void restream() override
    {
        if (shown) {
            showForClient(*player, true);
        }
    }

    void setText(StringView txt) override
    {
        TextDrawBase<IPlayerTextDraw>::setText(txt);
        if (shown) {
            setTextForClient(*player, txt, true);
        }
    }

    ~PlayerTextDraw()
    {
        if (player) {
            hideForClient(*player, true);
        }
    }
};
