#pragma once

#include <sdk.hpp>

struct IVehicle;

/// Object material text align values
enum ObjectMaterialTextAlign {
    ObjectMaterialTextAlign_Left,
    ObjectMaterialTextAlign_Center,
    ObjectMaterialTextAlign_Right
};

/// Object selection type
enum ObjectSelectType {
    ObjectSelectType_None,
    ObjectSelectType_Global,
    ObjectSelectType_Player
};

/// Object edit responses
enum ObjectEditResponse {
    ObjectEditResponse_Cancel,
    ObjectEditResponse_Final,
    ObjectEditResponse_Update
};

/// Player bones for attaching objects
enum PlayerBone {
    PlayerBone_None,
    PlayerBone_Spine,
    PlayerBone_Head,
    PlayerBone_LeftUpperArm,
    PlayerBone_RightUpperArm,
    PlayerBone_LeftHand,
    PlayerBone_RightHand,
    PlayerBone_LeftThigh,
    PlayerBone_RightThigh,
    PlayerBone_LeftFoot,
    PlayerBone_RightFoot,
    PlayerBone_RightCalf,
    PlayerBone_LeftCalf,
    PlayerBone_LeftForearm,
    PlayerBone_RightForearm,
    PlayerBone_LeftShoulder,
    PlayerBone_RightShoulder,
    PlayerBone_Neck,
    PlayerBone_Jaw
};

/* Interfaces, to be passed around */

/// Trivial object material data
struct ObjectMaterialData {
    enum Type {
        None,
        Default,
        Text
    };

    Type type; // Shared

    union {
        int model; // Default
        struct { // Text
            uint8_t materialSize;
            uint8_t fontSize;
            uint8_t alignment;
            bool bold;
        };
    };

    union {
        Colour materialColour; // Default
        Colour fontColour; // Text
    };

    Colour backgroundColour; // Text
};

/// Object material data
struct IObjectMaterial {
    /// Get various data of the material
    virtual const ObjectMaterialData& getData() const = 0;

    /// Get the material's TXD name (for type Default) or text (for type Text)
    virtual StringView getTXDOrText() const = 0;

    /// Get the material's texture name (for type Default) or font name (for type Text)
    virtual StringView getTextureOrFont() const = 0;
};

/// Object attachment data
struct ObjectAttachmentData {
    enum class Type {
        None,
        Vehicle,
        Object,
        Player
    } type;
    int ID;
    Vector3 offset;
    Vector3 rotation;
    bool syncRotation;
};

/// Player slot object attachment data
struct ObjectAttachmentSlotData {
    int model;
    int bone;
    Vector3 offset;
    Vector3 rotation;
    Vector3 scale;
    Colour colour1;
    Colour colour2;
};

/// Object move data
struct ObjectMoveData {
    Vector3 targetPos;
    Vector3 targetRot;
    float speed;
};

/// Base object class
struct IBaseObject : public IEntity {
    /// Set the draw distance of the object
    virtual void setDrawDistance(float drawDistance) = 0;

    /// Get the object's draw distance
    virtual float getDrawDistance() const = 0;

    /// Set the model of the object
    virtual void setModel(int model) = 0;

    /// Get the object's model
    virtual int getModel() const = 0;

    /// Set whether the object has camera collision
    virtual void setCameraCollision(bool collision) = 0;

    /// Get whether the object has camera collision
    virtual bool getCameraCollision() const = 0;

    /// Start moving the object
    virtual void startMoving(const ObjectMoveData& data) = 0;

    /// Get whether the object is moving
    virtual bool isMoving() const = 0;

    /// Stop moving the object prematurely
    virtual void stopMoving() = 0;

    /// Attach the object to a vehicle
    virtual void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) = 0;

    /// Reset any attachment data about the object
    virtual void resetAttachment() = 0;

    /// Get the object's attachment data
    virtual const ObjectAttachmentData& getAttachmentData() const = 0;

    /// Get the object's material data
    virtual bool getMaterialData(int index, const IObjectMaterial*& out) const = 0;

    /// Set the object's material to a texture
    virtual void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) = 0;

    /// Set the object's material to some text
    virtual void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) = 0;
};

/// An object interface
struct IObject : public IBaseObject {
    /// Attach the object to a player
    virtual void attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation) = 0;

    /// Attach the object to another object
    virtual void attachToObject(IObject& object, Vector3 offset, Vector3 rotation, bool syncRotation) = 0;
};

struct IPlayerObject : public IBaseObject {
    virtual void attachToObject(IPlayerObject& object, Vector3 offset, Vector3 rotation) = 0;
};

struct ObjectEventHandler;

/// An object pool
static const UUID ObjectsComponent_UUID = UUID(0x59f8415f72da6160);
struct IObjectsComponent : public IPoolComponent<IObject, OBJECT_POOL_SIZE> {
    PROVIDE_UUID(ObjectsComponent_UUID)

    /// Get the object event dispatcher
    virtual IEventDispatcher<ObjectEventHandler>& getEventDispatcher() = 0;

    /// Set the default camera collision for new objects
    virtual void setDefaultCameraCollision(bool collision) = 0;

    /// Get the default camera collision for new objects
    virtual bool getDefaultCameraCollision() const = 0;

    /// Create a new object
    /// @return A pointer if succeeded or nullptr on failure
    virtual IObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist = 0.f) = 0;
};

/// An object event handler
struct ObjectEventHandler {
    virtual void onMoved(IObject& object) { }
    virtual void onPlayerObjectMoved(IPlayer& player, IPlayerObject& object) { }
    virtual void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) { }
    virtual void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) { }
    virtual void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) { }
    virtual void onPlayerObjectEdited(IPlayer& player, IPlayerObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) { }
    virtual void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) { }
};

static const UUID PlayerObjectData_UUID = UUID(0x93d4ed2344b07456);
struct IPlayerObjectData : public IPlayerData, public IPool<IPlayerObject, OBJECT_POOL_SIZE> {
    PROVIDE_UUID(PlayerObjectData_UUID);

    /// Create a new player object
    /// @return A pointer if succeeded or nullptr on failure
    virtual IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist = 0.f) = 0;

    /// Set the player's attached object in an attachment slot
    virtual void setAttachedObject(int index, const ObjectAttachmentSlotData& data) = 0;

    /// Remove the player's attached object in an attachment slot
    virtual void removeAttachedObject(int index) = 0;

    /// Check if the player has an attached object in an attachment slot
    virtual bool hasAttachedObject(int index) const = 0;

    /// Get the player's attached object in an attachment slot
    virtual const ObjectAttachmentSlotData& getAttachedObject(int index) const = 0;

    /// Initiate object selection for the player
    virtual void beginObjectSelection() = 0;

    /// Get whether the player is selecting objects
    virtual bool selectingObject() const = 0;

    /// End selection and editing objects for the player
    virtual void endObjectEdit() = 0;

    /// Edit the object for the player
    virtual void editObject(IObject& object) = 0;

    /// Edit the player object for the player
    virtual void editObject(IPlayerObject& object) = 0;

    /// Check if the player is editing an object
    virtual bool editingObject() const = 0;

    /// Edit an attached object in an attachment slot for the player
    virtual void editAttachedObject(int index) = 0;
};

/* Implementation, NOT to be passed around */

/// Default object material data implementation
struct ObjectMaterial : public IObjectMaterial {
    String txdOrText;
    String textureOrFont;
    ObjectMaterialData data;

    const ObjectMaterialData& getData() const override
    {
        return data;
    }

    StringView getTXDOrText() const override
    {
        return txdOrText;
    }

    StringView getTextureOrFont() const override
    {
        return textureOrFont;
    }
};
