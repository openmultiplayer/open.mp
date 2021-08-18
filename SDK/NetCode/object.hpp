#pragma once

#include "../types.hpp"
#include "../network.hpp"
#include "../player.hpp"
#include "../Server/Components/Objects/objects.hpp"

namespace NetCode {
	namespace RPC {
		struct SetPlayerObjectMaterial final : NetworkPacketBase<84> {
			int ObjectID;
			int MaterialID;
			IObjectMaterial const & MaterialData;

			SetPlayerObjectMaterial(IObjectMaterial const & materialData) : MaterialData(materialData)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::UINT8(MaterialData.getData().type));
				bs.write(NetworkBitStreamValue::UINT8(MaterialID));

				if (MaterialData.getData().type == ObjectMaterialData::Type::Default) {
					bs.write(NetworkBitStreamValue::UINT16(MaterialData.getData().model));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(MaterialData.getTXD()));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(MaterialData.getTexture()));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.getData().materialColour.ARGB()));
				}
				else if (MaterialData.getData().type == ObjectMaterialData::Type::Text) {
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.getData().materialSize));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(MaterialData.getFont()));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.getData().fontSize));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.getData().bold));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.getData().fontColour.ARGB()));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.getData().backgroundColour.ARGB()));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.getData().alignment));
					bs.write(NetworkBitStreamValue::COMPRESSED_STR(MaterialData.getText()));
				}
			}
		};

		struct CreateObject final : NetworkPacketBase<44> {
			int ObjectID;
			int ModelID;
			Vector3 Position;
			Vector3 Rotation;
			float DrawDistance;
			bool CameraCollision;
			ObjectAttachmentData AttachmentData;
			StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS> Materials;
			StaticBitset<MAX_OBJECT_MATERIAL_SLOTS>& MaterialsUsed;

			CreateObject(
				StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS>& materials,
				StaticBitset<MAX_OBJECT_MATERIAL_SLOTS>& materialsUsed
			) :
				Materials(materials),
				MaterialsUsed(materialsUsed)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::INT32(ModelID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
				bs.write(NetworkBitStreamValue::FLOAT(DrawDistance));
				bs.write(NetworkBitStreamValue::UINT8(CameraCollision));

				bs.write(NetworkBitStreamValue::UINT16(AttachmentData.type == ObjectAttachmentData::Type::Vehicle ? AttachmentData.ID : INVALID_VEHICLE_ID));
				bs.write(NetworkBitStreamValue::UINT16(AttachmentData.type == ObjectAttachmentData::Type::Object ? AttachmentData.ID : INVALID_OBJECT_ID));
				if (AttachmentData.type == ObjectAttachmentData::Type::Vehicle || AttachmentData.type == ObjectAttachmentData::Type::Object) {
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.offset));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.rotation));
					bs.write(NetworkBitStreamValue::UINT8(AttachmentData.syncRotation));
				}

				bs.write(NetworkBitStreamValue::UINT8(MaterialsUsed.count()));
				for (int i = 0; i < MaterialsUsed.count(); ++i) {
					if (MaterialsUsed.test(i)) {
						ObjectMaterial const & material = Materials[i];
						bs.write(NetworkBitStreamValue::UINT8(material.getData().type));
						bs.write(NetworkBitStreamValue::UINT8(i));

						if (material.getData().type == ObjectMaterialData::Type::Default) {
							bs.write(NetworkBitStreamValue::UINT16(material.getData().model));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(material.getTXD()));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(material.getTexture()));
							bs.write(NetworkBitStreamValue::UINT32(material.getData().materialColour.ARGB()));
						}
						else if (material.getData().type == ObjectMaterialData::Type::Text) {
							bs.write(NetworkBitStreamValue::UINT8(material.getData().materialSize));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(material.getFont()));
							bs.write(NetworkBitStreamValue::UINT8(material.getData().fontSize));
							bs.write(NetworkBitStreamValue::UINT8(material.getData().bold));
							bs.write(NetworkBitStreamValue::UINT32(material.getData().fontColour.ARGB()));
							bs.write(NetworkBitStreamValue::UINT32(material.getData().backgroundColour.ARGB()));
							bs.write(NetworkBitStreamValue::UINT8(material.getData().alignment));
							bs.write(NetworkBitStreamValue::COMPRESSED_STR(material.getText()));
						}
					}
				}
			}
		};

		struct DestroyObject final : NetworkPacketBase<47> {
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct MoveObject final : NetworkPacketBase<99> {
			int ObjectID;
			Vector3 CurrentPosition;
			ObjectMoveData MoveData;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(CurrentPosition));
				bs.write(NetworkBitStreamValue::VEC3(MoveData.targetPos));
				bs.write(NetworkBitStreamValue::FLOAT(MoveData.speed));
				bs.write(NetworkBitStreamValue::VEC3(MoveData.targetRot));
			}
		};

		struct StopObject final : NetworkPacketBase<122> {
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct SetObjectPosition final : NetworkPacketBase<45> {
			int ObjectID;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
			}
		};

		struct SetObjectRotation final : NetworkPacketBase<46> {
			int ObjectID;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
			}
		};

		struct AttachObjectToPlayer final : NetworkPacketBase<75> {
			int ObjectID;
			int PlayerID;
			Vector3 Offset;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::VEC3(Offset));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
			}
		};

		struct SetPlayerAttachedObject final : NetworkPacketBase<113> {
			int PlayerID;
			int Index;
			bool Create;
			ObjectAttachmentSlotData AttachmentData;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Index));
				bs.write(NetworkBitStreamValue::BIT(Create));
				if (Create) {
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.model));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.bone));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.offset));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.rotation));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.scale));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.colour1.ARGB()));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.colour2.ARGB()));
				}
			}
		};

		struct PlayerBeginObjectSelect final : NetworkPacketBase<27> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct OnPlayerSelectObject final : NetworkPacketBase<27> {
			int SelectType;
			int ObjectID;
			int Model;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT32>(SelectType);
				bs.read<NetworkBitStreamValueType::UINT16>(ObjectID);
				bs.read<NetworkBitStreamValueType::UINT32>(Model);
				return bs.read<NetworkBitStreamValueType::VEC3>(Position);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerCancelObjectEdit final : NetworkPacketBase<28> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerBeginObjectEdit final : NetworkPacketBase<117> {
			bool PlayerObject;
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(PlayerObject));
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct OnPlayerEditObject final : NetworkPacketBase<117> {
			bool PlayerObject;
			int ObjectID;
			int Response;
			Vector3 Offset;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::BIT>(PlayerObject);
				bs.read<NetworkBitStreamValueType::UINT16>(ObjectID);
				bs.read<NetworkBitStreamValueType::UINT32>(Response);
				bs.read<NetworkBitStreamValueType::VEC3>(Offset);
				return bs.read<NetworkBitStreamValueType::VEC3>(Rotation);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerBeginAttachedObjectEdit final : NetworkPacketBase<116> {
			unsigned Index;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Index));
			}
		};

		struct OnPlayerEditAttachedObject final : NetworkPacketBase<116> {
			unsigned Response;
			unsigned Index;
			ObjectAttachmentSlotData AttachmentData;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT32>(Response);
				bs.read<NetworkBitStreamValueType::UINT32>(Index);
				bs.read<NetworkBitStreamValueType::UINT32>(AttachmentData.model);
				bs.read<NetworkBitStreamValueType::UINT32>(AttachmentData.bone);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.offset);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.rotation);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.scale);
				uint32_t argb;
				bs.read<NetworkBitStreamValueType::UINT32>(argb);
				AttachmentData.colour1 = Colour::FromARGB(argb);
				bool res = bs.read<NetworkBitStreamValueType::UINT32>(argb);
				AttachmentData.colour2 = Colour::FromARGB(argb);
				return res;
			}

			void write(INetworkBitStream& bs) const {
			}
		};
	}
}