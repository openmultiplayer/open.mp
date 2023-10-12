/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

template <class T>
class TextDrawBase : public T, public PoolIDProvider, public NoCopy
{
private:
	Vector2 pos;
	HybridString<64> text;
	Colour letterColour = Colour(0xE1, 0xE1, 0xE1);
	Vector2 letterSize = Vector2(0.48f, 1.12f);
	Vector2 textSize = Vector2(1280.f);
	TextDrawAlignmentTypes alignment = TextDrawAlignment_Default;
	bool box = false;
	bool proportional = true;
	bool selectable = false;
	Colour boxColour = Colour(0x80, 0x80, 0x80, 0x80);
	int shadowSize = 2;
	int outlineSize = 0;
	Colour backgroundColour = Colour::Black();
	TextDrawStyle style;
	int previewModel;
	Vector3 previewRotation = Vector3(0.f);
	Pair<int, int> previewVehicleColours = std::make_pair(-1, -1);
	float previewZoom = 1.f;

public:
	TextDrawBase(Vector2 pos, StringView text, TextDrawStyle style = TextDrawStyle_FontAharoniBold, int previewModel = 0)
		: pos(pos)
		, text(text)
		, style(style)
		, previewModel(previewModel)
	{
		trimText();
	}

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
		trimText();
	}

	StringView getText() const override
	{
		return text;
	}

	T& setColour(Colour col) override
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

	T& useBox(bool use) override
	{
		box = use;
		return *this;
	}

	bool hasBox() const override
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

	T& setBackgroundColour(Colour colour) override
	{
		backgroundColour = colour;
		return *this;
	}

	Colour getBackgroundColour() const override
	{
		return backgroundColour;
	}

	T& setStyle(TextDrawStyle s) override
	{
		if (static_cast<int>(s) >= 16 || static_cast<int>(s) < 0)
		{
			style = TextDrawStyle_FontBeckettRegular;
			return *this;
		}
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

	T& setPreviewRotation(Vector3 rotation) override
	{
		previewRotation = rotation;
		return *this;
	}

	Vector3 getPreviewRotation() const override
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

protected:
	void showForClient(IPlayer& player, bool isPlayerTextDraw)
	{
		NetCode::RPC::PlayerShowTextDraw playerShowTextDrawRPC;
		playerShowTextDrawRPC.PlayerTextDraw = isPlayerTextDraw;
		playerShowTextDrawRPC.UseBox = box;
		switch (alignment)
		{
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
		playerShowTextDrawRPC.BackgroundColour = backgroundColour;
		playerShowTextDrawRPC.Style = style;
		playerShowTextDrawRPC.Selectable = selectable;
		playerShowTextDrawRPC.Position = pos;
		playerShowTextDrawRPC.Model = previewModel;
		playerShowTextDrawRPC.Rotation = previewRotation;
		playerShowTextDrawRPC.Zoom = previewZoom;
		playerShowTextDrawRPC.Color1 = previewVehicleColours.first;
		playerShowTextDrawRPC.Color2 = previewVehicleColours.second;
		playerShowTextDrawRPC.Text = StringView(text);
		PacketHelper::send(playerShowTextDrawRPC, player);
	}

	void hideForClient(IPlayer& player, bool isPlayerTextDraw)
	{
		NetCode::RPC::PlayerHideTextDraw playerHideTextDrawRPC;
		playerHideTextDrawRPC.PlayerTextDraw = isPlayerTextDraw;
		playerHideTextDrawRPC.TextDrawID = poolID;
		PacketHelper::send(playerHideTextDrawRPC, player);
	}

	void setTextForClient(IPlayer& player, StringView txt, bool isPlayerTextDraw)
	{
		NetCode::RPC::PlayerTextDrawSetString playerTextDrawSetStringRPC;
		playerTextDrawSetStringRPC.PlayerTextDraw = isPlayerTextDraw;
		playerTextDrawSetStringRPC.TextDrawID = poolID;
		playerTextDrawSetStringRPC.Text = txt;
		PacketHelper::send(playerTextDrawSetStringRPC, player);
	}

	// Remove ending spaces. Set text length to client limit.
	void trimText()
	{
		String newText(text.data(), text.length());

		if (newText.length() >= MAX_TEXTDRAW_STR_LENGTH)
		{
			newText.resize(MAX_TEXTDRAW_STR_LENGTH - 1);
		}

		while (newText.length() && newText.back() == ' ')
		{
			newText.pop_back();
		}

		text = newText;
	}
};

class TextDraw final : public TextDrawBase<ITextDraw>
{
private:
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> shownFor_;

	using TextDrawBase<ITextDraw>::TextDrawBase;

public:
	void removeFor(int pid, IPlayer& player)
	{
		if (shownFor_.valid(pid))
		{
			shownFor_.remove(pid, player);
		}
	}

	void restream() override
	{
		for (IPlayer* player : shownFor_.entries())
		{
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
		for (IPlayer* player : shownFor_.entries())
		{
			setTextForClient(*player, txt, false);
		}
	}

	void setTextForPlayer(IPlayer& player, StringView txt) override
	{
		setTextForClient(player, txt, false);
	}

	~TextDraw()
	{
	}

	void destream()
	{
		for (IPlayer* player : shownFor_.entries())
		{
			hideForClient(*player, false);
		}
	}
};

class PlayerTextDraw final : public TextDrawBase<IPlayerTextDraw>
{
private:
	IPlayer& player;
	bool shown = false;

public:
	PlayerTextDraw(IPlayer& player, Vector2 pos, StringView text, TextDrawStyle style = TextDrawStyle_FontAharoniBold, int previewModel = 0)
		: TextDrawBase(pos, text, style, previewModel)
		, player(player)
	{
	}

	void show() override
	{
		showForClient(player, true);
		shown = true;
	}

	void hide() override
	{
		hideForClient(player, true);
		shown = false;
	}

	bool isShown() const override
	{
		return shown;
	}

	void restream() override
	{
		if (shown)
		{
			showForClient(player, true);
		}
	}

	void setText(StringView txt) override
	{
		TextDrawBase<IPlayerTextDraw>::setText(txt);
		if (shown)
		{
			setTextForClient(player, txt, true);
		}
	}

	~PlayerTextDraw()
	{
	}

	void destream()
	{
		if (shown)
		{
			hideForClient(player, true);
		}
	}
};
