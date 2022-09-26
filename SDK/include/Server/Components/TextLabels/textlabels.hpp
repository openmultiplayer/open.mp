#pragma once

#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

struct IVehicle;

/// Text label attachment data
struct TextLabelAttachmentData
{
	int playerID = INVALID_PLAYER_ID;
	int vehicleID = INVALID_VEHICLE_ID;
};

/// Text label base interface
struct ITextLabelBase : public IExtensible, public IEntity
{
	/// Set the text label's text
	virtual void setText(StringView text) = 0;

	/// Get the text label's text
	virtual StringView getText() const = 0;

	/// Set the text label's colour
	virtual void setColour(Colour colour) = 0;

	/// Get the text label's colour
	virtual Colour getColour() const = 0;

	/// Set the text label's draw distance
	virtual void setDrawDistance(float dist) = 0;

	/// Get the text label's draw distance
	virtual float getDrawDistance() = 0;

	/// Attach the text label to a player with an offset
	virtual void attachToPlayer(IPlayer& player, Vector3 offset) = 0;

	/// Attach the text label to a vehicle with an offset
	virtual void attachToVehicle(IVehicle& vehicle, Vector3 offset) = 0;

	/// Get the text label's attachment data
	virtual const TextLabelAttachmentData& getAttachmentData() const = 0;

	/// Detach the text label from the player and set its position or offset
	virtual void detachFromPlayer(Vector3 position) = 0;

	/// Detach the text label from the vehicle and set its position or offset
	virtual void detachFromVehicle(Vector3 position) = 0;

	// Set the text label's los check.
	virtual void setTestLOS(bool status) = 0;

	// Get the text label's los check status.
	virtual bool getTestLOS() const = 0;

	// Used to update both colour and text with one single network packet being sent.
	virtual void setColourAndText(Colour colour, StringView text) = 0;
};

/// A global text label
struct ITextLabel : public ITextLabelBase
{
	/// Checks if player has the text label streamed in for themselves
	virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

	/// Streams in the text label for a specific player
	virtual void streamInForPlayer(IPlayer& player) = 0;

	/// Streams out the text label for a specific player
	virtual void streamOutForPlayer(IPlayer& player) = 0;
};

/// A player text label
struct IPlayerTextLabel : public ITextLabelBase
{
};

static const UID TextLabelsComponent_UID = UID(0xa0c57ea80a009742);
/// The text label component which is a global text label pool
struct ITextLabelsComponent : public IPoolComponent<ITextLabel>
{
	PROVIDE_UID(TextLabelsComponent_UID);

	/// Create a text label
	virtual ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los) = 0;

	/// Create a text label and attach it to a player
	virtual ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IPlayer& attach) = 0;

	/// Create a text label and attach it to a vehicle
	virtual ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IVehicle& attach) = 0;
};

static const UID PlayerTextLabelData_UID = UID(0xb9e2bd0dc5148c3c);
/// The text label player data which is a player text label pool
struct IPlayerTextLabelData : public IExtension, public IPool<IPlayerTextLabel>
{
	PROVIDE_EXT_UID(PlayerTextLabelData_UID);

	/// Create a player text label
	virtual IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) = 0;

	/// Create a player text label and attach it to a player
	virtual IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IPlayer& attach) = 0;

	/// Create a player text label and attach it to a vehicle
	virtual IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IVehicle& attach) = 0;
};
