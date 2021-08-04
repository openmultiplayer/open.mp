#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "sdk.hpp"
#include "Impl.hpp"
#include "../Manager/Manager.hpp"

/// Macro to define a script param for a pool entry
/// Example with IPlayer from the players pool:
/// Using IPlayer& in a script function throws an exception if the player with the specified ID doesn't exist
/// Using IPlayer* in a script function makes the IPlayer entry optional and is nullptr if the player with the specified ID doesn't exist
#define POOL_PARAM(type, poolPtr) \
	template <> \
	struct ParamLookup<type> { \
		static type& ValReq(cell ref) { \
			auto pool = PawnManager::Get()->poolPtr; \
			if (pool && pool->valid(ref)) { \
				return pool->get(ref); \
			} \
			throw pawn_natives::ParamCastFailure(); \
		} \
 \
		static type* Val(cell ref) { \
			auto pool = PawnManager::Get()->poolPtr; \
			if (pool && pool->valid(ref)) { \
				return &pool->get(ref); \
			} \
			return nullptr; \
		} \
	}; \
 \
	template <> \
	class ParamCast<type*> { \
	public: \
		ParamCast(AMX* amx, cell* params, int idx) { \
			value_ = ParamLookup<type>::Val(params[idx]); \
		} \
 \
		~ParamCast() \
		{} \
 \
		ParamCast(ParamCast<type*> const&) = delete; \
		ParamCast(ParamCast<type*>&&) = delete; \
 \
		operator type* () { \
			return value_; \
		} \
 \
		static constexpr int Size = 1; \
 \
	private: \
		type* value_; \
	}; \
 \
	template <> \
	class ParamCast<type&> { \
	public: \
		ParamCast(AMX* amx, cell* params, int idx) : \
			value_(ParamLookup<type>::ValReq(params[idx])) \
		{} \
 \
		~ParamCast() \
		{} \
 \
		ParamCast(ParamCast<type&> const&) = delete; \
		ParamCast(ParamCast<type&>&&) = delete; \
 \
		operator type& () { \
			return value_; \
		} \
 \
		static constexpr int Size = 1; \
 \
	private: \
		type& value_; \
	}; \
\
	template <> \
	class ParamCast<const type&> \
	{ \
	public: \
		ParamCast(AMX*, cell*, int) = delete; \
		ParamCast() = delete; \
	};

/// Macro to define a script param for a player pool entry
/// Always throws if the player ID specified in the first parameter is invalid as it's required to get the IPlayerObjectData pool
/// Example with IPlayerObject from the IPlayerObjectData pool:
/// Using IPlayerObject& in a script function throws an exception if the object with the specified ID doesn't exist
/// Using IPlayerObject* in a script function makes the IPlayerObject entry optional and is nullptr if the object with the specified ID doesn't exist
#define PLAYER_POOL_PARAM(type, dataType) \
	template <> \
	struct ParamLookup<type> { \
		static type& ValReq(IPlayer& player, cell ref) { \
			auto data = player.queryData<dataType>(); \
			if (data && data->valid(ref)) { \
				return data->get(ref); \
			} \
			throw pawn_natives::ParamCastFailure(); \
		} \
 \
		static type* Val(IPlayer& player, cell ref) { \
			auto data = player.queryData<dataType>(); \
			if (data && data->valid(ref)) { \
				return &data->get(ref); \
			} \
			return nullptr; \
		} \
	}; \
 \
	template <> \
	class ParamCast<type*> { \
	public: \
		ParamCast(AMX* amx, cell* params, int idx) { \
			value_ = ParamLookup<type>::Val(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx]); \
		} \
 \
		~ParamCast() \
		{} \
 \
		ParamCast(ParamCast<type*> const&) = delete; \
		ParamCast(ParamCast<type*>&&) = delete; \
 \
		operator type* () { \
			return value_; \
		} \
 \
		static constexpr int Size = 1; \
 \
	private: \
		type* value_; \
	}; \
 \
	template <> \
	class ParamCast<type&> { \
	public: \
		ParamCast(AMX* amx, cell* params, int idx) : \
			value_(ParamLookup<type>::ValReq(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx])) \
		{} \
 \
		~ParamCast() \
		{} \
 \
		ParamCast(ParamCast<type&> const&) = delete; \
		ParamCast(ParamCast<type&>&&) = delete; \
 \
		operator type& () { \
			return value_; \
		} \
 \
		static constexpr int Size = 1; \
 \
	private: \
		type& value_; \
	}; \
\
	template <> \
	class ParamCast<const type&> \
	{ \
	public: \
		ParamCast(AMX*, cell*, int) = delete; \
		ParamCast() = delete; \
	};

// custom ParamCasts here to use custom types in native declarations
namespace pawn_natives
{
	POOL_PARAM(IPlayer, players);
	POOL_PARAM(IActor, actors);
	POOL_PARAM(PlayerClass, classes);
	POOL_PARAM(IGangZone, gangzones);
	POOL_PARAM(IMenu, menus);
	POOL_PARAM(IObject, objects);
	POOL_PARAM(IPickup, pickups);
	POOL_PARAM(ITextDraw, textdraws);
	POOL_PARAM(ITextLabel, textlabels);
	POOL_PARAM(IVehicle, vehicles);

	PLAYER_POOL_PARAM(IPlayerObject, IPlayerObjectData);
	PLAYER_POOL_PARAM(IPlayerTextDraw, IPlayerTextDrawData);
	PLAYER_POOL_PARAM(IPlayerTextLabel, IPlayerTextLabelData);

	// Disable the value version.
	template <>
	class ParamCast<glm::vec3>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec3 &>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec3 const &>
	{
	public:
		ParamCast([[maybe_unused]] AMX * amx, cell * params, int idx)
			:
			value_{ amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]) }
		{
		}

		~ParamCast()
		{
		}

		operator glm::vec3 const & () const
		{
			return value_;
		}

		static constexpr int Size = 3;

		using type = glm::vec3 const &;

	private:
		glm::vec3
			value_;
	};

	template <>
	class ParamCast<glm::vec3 *>
	{
	public:
		ParamCast(AMX * amx, cell * params, int idx)
		{
			amx_GetAddr(amx, params[idx + 0], &x_);
			amx_GetAddr(amx, params[idx + 1], &y_);
			amx_GetAddr(amx, params[idx + 2], &z_);
			value_.x = amx_ctof(*x_);
			value_.y = amx_ctof(*y_);
			value_.z = amx_ctof(*z_);
		}

		~ParamCast()
		{
			// Write the value back in to memory.
			*x_ = amx_ftoc(value_.x);
			*y_ = amx_ftoc(value_.y);
			*z_ = amx_ftoc(value_.z);
		}

		operator glm::vec3 * ()
		{
			return &value_;
		}

		static constexpr int Size = 3;

		using type = glm::vec3 *;

	private:
		glm::vec3
			value_;

		cell
			* x_,
			* y_,
			* z_;
	};

	template <>
	class ParamCast<glm::vec2>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec2 &>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec2 const &>
	{
	public:
		ParamCast([[maybe_unused]] AMX * amx, cell * params, int idx)
			:
			value_{ amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]) }
		{
		}

		~ParamCast()
		{
		}

		operator glm::vec2 const & () const
		{
			return value_;
		}

		static constexpr int Size = 2;

		using type = glm::vec2 const &;

	private:
		glm::vec2
			value_;
	};

	template <>
	class ParamCast<glm::vec2 *>
	{
	public:
		ParamCast(AMX * amx, cell * params, int idx)
		{
			amx_GetAddr(amx, params[idx + 0], &x_);
			amx_GetAddr(amx, params[idx + 1], &y_);
			value_.x = amx_ctof(*x_);
			value_.y = amx_ctof(*y_);
		}

		~ParamCast()
		{
			// Write the value back in to memory.
			*x_ = amx_ftoc(value_.x);
			*y_ = amx_ftoc(value_.y);
		}

		operator glm::vec2 * ()
		{
			return &value_;
		}

		static constexpr int Size = 2;

		using type = glm::vec2 *;

	private:
		glm::vec2
			value_;

		cell
			* x_,
			* y_;
	};

	template <>
	class ParamCast<glm::vec4>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec4 &>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::vec4 const &>
	{
	public:
		ParamCast([[maybe_unused]] AMX * amx, cell * params, int idx)
			:
			value_{ amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]), amx_ctof(params[idx + 3]) }
		{
		}

		~ParamCast()
		{
		}

		operator glm::vec4 const & () const
		{
			return value_;
		}

		static constexpr int Size = 4;

		using type = glm::vec4 const &;

	private:
		glm::vec4
			value_;
	};

	template <>
	class ParamCast<glm::vec4 *>
	{
	public:
		ParamCast(AMX * amx, cell * params, int idx)
		{
			amx_GetAddr(amx, params[idx + 0], &x_);
			amx_GetAddr(amx, params[idx + 1], &y_);
			amx_GetAddr(amx, params[idx + 2], &z_);
			amx_GetAddr(amx, params[idx + 3], &w_);
			value_.x = amx_ctof(*x_);
			value_.y = amx_ctof(*y_);
			value_.z = amx_ctof(*z_);
			value_.w = amx_ctof(*w_);
		}

		~ParamCast()
		{
			// Write the value back in to memory.
			*x_ = amx_ftoc(value_.x);
			*y_ = amx_ftoc(value_.y);
			*z_ = amx_ftoc(value_.z);
			*w_ = amx_ftoc(value_.w);
		}

		operator glm::vec4 * ()
		{
			return &value_;
		}

		static constexpr int Size = 4;

		using type = glm::vec4 *;

	private:
		glm::vec4
			value_;

		cell
			* x_,
			* y_,
			* z_,
			* w_;
	};

	template <>
	class ParamCast<glm::quat>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::quat &>
	{
	public:
		ParamCast(AMX *, cell *, int) = delete;
		ParamCast() = delete;
	};

	template <>
	class ParamCast<glm::quat const &>
	{
	public:
		ParamCast([[maybe_unused]] AMX * amx, cell * params, int idx)
			:
			value_{ amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]), amx_ctof(params[idx + 3]) }
		{
		}

		~ParamCast()
		{
		}

		operator glm::quat const & () const
		{
			return value_;
		}

		static constexpr int Size = 4;

		using type = glm::quat const &;

	private:
		glm::quat
			value_;
	};

	template <>
	class ParamCast<glm::quat *>
	{
	public:
		ParamCast(AMX * amx, cell * params, int idx)
		{
			amx_GetAddr(amx, params[idx + 0], &w_);
			amx_GetAddr(amx, params[idx + 1], &x_);
			amx_GetAddr(amx, params[idx + 2], &y_);
			amx_GetAddr(amx, params[idx + 3], &z_);
			value_.w = amx_ctof(*w_);
			value_.x = amx_ctof(*x_);
			value_.y = amx_ctof(*y_);
			value_.z = amx_ctof(*z_);
		}

		~ParamCast()
		{
			// Write the value back in to memory.
			*w_ = amx_ftoc(value_.w);
			*x_ = amx_ftoc(value_.x);
			*y_ = amx_ftoc(value_.y);
			*z_ = amx_ftoc(value_.z);
		}

		operator glm::quat * ()
		{
			return &value_;
		}

		static constexpr int Size = 4;

		using type = glm::quat *;

	private:
		glm::quat
			value_;

		cell
			* w_,
			* x_,
			* y_,
			* z_;
	};
}