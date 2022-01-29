#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "../Manager/Manager.hpp"
#include "Impl.hpp"
#include "sdk.hpp"

/// Macro to define a script param for a pool entry
/// Example with IPlayer from the players pool:
/// Using IPlayer& in a script function throws an exception if the player with the specified ID doesn't exist
/// Using IPlayer* in a script function makes the IPlayer entry optional and is nullptr if the player with the specified ID doesn't exist
#define POOL_PARAM(type, poolPtr)                            \
    template <>                                              \
    struct ParamLookup<type> {                               \
        static type& ValReq(cell ref)                        \
        {                                                    \
            auto pool = PawnManager::Get()->poolPtr;         \
            if (pool) {                                      \
                auto ptr = pool->get(ref);                   \
                if (ptr) {                                   \
                    return *ptr;                             \
                }                                            \
            }                                                \
            throw pawn_natives::ParamCastFailure();          \
        }                                                    \
                                                             \
        static type* Val(cell ref) noexcept                  \
        {                                                    \
            auto pool = PawnManager::Get()->poolPtr;         \
            if (pool) {                                      \
                return pool->get(ref);                       \
            }                                                \
            return nullptr;                                  \
        }                                                    \
    };                                                       \
                                                             \
    template <>                                              \
    class ParamCast<type*> {                                 \
    public:                                                  \
        ParamCast(AMX* amx, cell* params, int idx) noexcept  \
        {                                                    \
            value_ = ParamLookup<type>::Val(params[idx]);    \
        }                                                    \
                                                             \
        ~ParamCast()                                         \
        {                                                    \
        }                                                    \
                                                             \
        ParamCast(ParamCast<type*> const&) = delete;         \
        ParamCast(ParamCast<type*>&&) = delete;              \
                                                             \
        operator type*()                                     \
        {                                                    \
            return value_;                                   \
        }                                                    \
                                                             \
        static constexpr int Size = 1;                       \
                                                             \
    private:                                                 \
        type* value_;                                        \
    };                                                       \
                                                             \
    template <>                                              \
    class ParamCast<type&> {                                 \
    public:                                                  \
        ParamCast(AMX* amx, cell* params, int idx)           \
            : value_(ParamLookup<type>::ValReq(params[idx])) \
        {                                                    \
        }                                                    \
                                                             \
        ~ParamCast()                                         \
        {                                                    \
        }                                                    \
                                                             \
        ParamCast(ParamCast<type&> const&) = delete;         \
        ParamCast(ParamCast<type&>&&) = delete;              \
                                                             \
        operator type&()                                     \
        {                                                    \
            return value_;                                   \
        }                                                    \
                                                             \
        static constexpr int Size = 1;                       \
                                                             \
    private:                                                 \
        type& value_;                                        \
    };                                                       \
                                                             \
    template <>                                              \
    class ParamCast<const type&> {                           \
    public:                                                  \
        ParamCast(AMX*, cell*, int) = delete;                \
        ParamCast() = delete;                                \
    };

/// Macro to define a script param for a player pool entry
/// Always throws if the player ID specified in the first parameter is invalid as it's required to get the IPlayerObjectData pool
/// Example with IPlayerObject from the IPlayerObjectData pool:
/// Using IPlayerObject& in a script function throws an exception if the object with the specified ID doesn't exist
/// Using IPlayerObject* in a script function makes the IPlayerObject entry optional and is nullptr if the object with the specified ID doesn't exist
#define PLAYER_POOL_PARAM(type, dataType)                                                                                                  \
    template <>                                                                                                                            \
    struct ParamLookup<type> {                                                                                                             \
        static type& ValReq(IPlayer& player, cell ref)                                                                                     \
        {                                                                                                                                  \
            auto data = queryData<dataType>(player);                                                                                       \
            if (data) {                                                                                                                    \
                auto ptr = data->get(ref);                                                                                                 \
                if (ptr) {                                                                                                                 \
                    return *ptr;                                                                                                           \
                }                                                                                                                          \
            }                                                                                                                              \
            throw pawn_natives::ParamCastFailure();                                                                                        \
        }                                                                                                                                  \
                                                                                                                                           \
        static type* Val(IPlayer& player, cell ref) noexcept                                                                               \
        {                                                                                                                                  \
            auto data = queryData<dataType>(player);                                                                                       \
            if (data) {                                                                                                                    \
                return data->get(ref);                                                                                                     \
            }                                                                                                                              \
            return nullptr;                                                                                                                \
        }                                                                                                                                  \
    };                                                                                                                                     \
                                                                                                                                           \
    template <>                                                                                                                            \
    class ParamCast<type*> {                                                                                                               \
    public:                                                                                                                                \
        ParamCast(AMX* amx, cell* params, int idx)                                                                                         \
        {                                                                                                                                  \
            value_ = ParamLookup<type>::Val(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx]);    \
        }                                                                                                                                  \
                                                                                                                                           \
        ~ParamCast()                                                                                                                       \
        {                                                                                                                                  \
        }                                                                                                                                  \
                                                                                                                                           \
        ParamCast(ParamCast<type*> const&) = delete;                                                                                       \
        ParamCast(ParamCast<type*>&&) = delete;                                                                                            \
                                                                                                                                           \
        operator type*()                                                                                                                   \
        {                                                                                                                                  \
            return value_;                                                                                                                 \
        }                                                                                                                                  \
                                                                                                                                           \
        static constexpr int Size = 1;                                                                                                     \
                                                                                                                                           \
    private:                                                                                                                               \
        type* value_;                                                                                                                      \
    };                                                                                                                                     \
                                                                                                                                           \
    template <>                                                                                                                            \
    class ParamCast<type const*> {                                                                                                         \
    public:                                                                                                                                \
        ParamCast(AMX* amx, cell* params, int idx) = delete;                                                                               \
        ParamCast() = delete;                                                                                                              \
    };                                                                                                                                     \
                                                                                                                                           \
    template <>                                                                                                                            \
    class ParamCast<type&> {                                                                                                               \
    public:                                                                                                                                \
        ParamCast(AMX* amx, cell* params, int idx)                                                                                         \
            : value_(ParamLookup<type>::ValReq(ParamLookup<IPlayer>::ValReq(params[1] /* first param is always playerid */), params[idx])) \
        {                                                                                                                                  \
        }                                                                                                                                  \
                                                                                                                                           \
        ~ParamCast()                                                                                                                       \
        {                                                                                                                                  \
        }                                                                                                                                  \
                                                                                                                                           \
        ParamCast(ParamCast<type&> const&) = delete;                                                                                       \
        ParamCast(ParamCast<type&>&&) = delete;                                                                                            \
                                                                                                                                           \
        operator type&()                                                                                                                   \
        {                                                                                                                                  \
            return value_;                                                                                                                 \
        }                                                                                                                                  \
                                                                                                                                           \
        static constexpr int Size = 1;                                                                                                     \
                                                                                                                                           \
    private:                                                                                                                               \
        type& value_;                                                                                                                      \
    };                                                                                                                                     \
                                                                                                                                           \
    template <>                                                                                                                            \
    class ParamCast<const type&> {                                                                                                         \
    public:                                                                                                                                \
        ParamCast(AMX*, cell*, int) = delete;                                                                                              \
        ParamCast() = delete;                                                                                                              \
    };

// custom ParamCasts here to use custom types in native declarations
namespace pawn_natives {
POOL_PARAM(IPlayer, players);
POOL_PARAM(IActor, actors);
POOL_PARAM(IClass, classes);
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

// Database IDatabaseConnection param lookups
template <>
struct ParamLookup<IDatabaseConnection> {
    static IDatabaseConnection& ValReq(cell ref)
    {
        IDatabasesComponent* databases_component = PawnManager::Get()->databases;
        if (databases_component && databases_component->isDatabaseConnectionIDValid(static_cast<int>(ref))) {
            return databases_component->getDatabaseConnectionByID(static_cast<int>(ref));
        }
        throw pawn_natives::ParamCastFailure();
    }

    static IDatabaseConnection* Val(cell ref) noexcept
    {
        IDatabasesComponent* databases_component = PawnManager::Get()->databases;
        IDatabaseConnection* connection = nullptr;
        if (databases_component && databases_component->isDatabaseConnectionIDValid(static_cast<int>(ref))) {
            connection = &databases_component->getDatabaseConnectionByID(static_cast<int>(ref));
        }
        return connection;
    }
};

template <>
class ParamCast<IDatabaseConnection&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
        : value_(ParamLookup<IDatabaseConnection>::ValReq(params[idx]))
    {
    }

    ~ParamCast()
    {
    }

    ParamCast(ParamCast<IDatabaseConnection&> const&) = delete;
    ParamCast(ParamCast<IDatabaseConnection&>&&) = delete;

    operator IDatabaseConnection&()
    {
        return value_;
    }

    static constexpr int Size = 1;

private:
    IDatabaseConnection& value_;
};

template <>
class ParamCast<IDatabaseConnection*> {
public:
    ParamCast(AMX* amx, cell* params, int idx) noexcept
    {
        value_ = ParamLookup<IDatabaseConnection>::Val(params[idx]);
    }

    ~ParamCast()
    {
    }

    ParamCast(ParamCast<IDatabaseConnection*> const&) = delete;
    ParamCast(ParamCast<IDatabaseConnection*>&&) = delete;

    operator IDatabaseConnection*()
    {
        return value_;
    }

    static constexpr int Size = 1;

private:
    IDatabaseConnection* value_;
};

// Database IDatabaseResultSet param lookups
template <>
struct ParamLookup<IDatabaseResultSet> {
    static IDatabaseResultSet& ValReq(cell ref)
    {
        IDatabasesComponent* databases_component = PawnManager::Get()->databases;
        if (databases_component && databases_component->isDatabaseResultSetIDValid(static_cast<int>(ref))) {
            return databases_component->getDatabaseResultSetByID(static_cast<int>(ref));
        }
        throw pawn_natives::ParamCastFailure();
    }

    static IDatabaseResultSet* Val(cell ref) noexcept
    {
        IDatabasesComponent* databases_component = PawnManager::Get()->databases;
        IDatabaseResultSet* resultSet = nullptr;
        if (databases_component && databases_component->isDatabaseResultSetIDValid(static_cast<int>(ref))) {
            resultSet = &databases_component->getDatabaseResultSetByID(static_cast<int>(ref));
        }
        return resultSet;
    }
};

template <>
class ParamCast<IDatabaseResultSet&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
        : value_(ParamLookup<IDatabaseResultSet>::ValReq(params[idx]))
    {
    }

    ~ParamCast()
    {
    }

    ParamCast(ParamCast<IDatabaseResultSet&> const&) = delete;
    ParamCast(ParamCast<IDatabaseResultSet&>&&) = delete;

    operator IDatabaseResultSet&()
    {
        return value_;
    }

    static constexpr int Size = 1;

private:
    IDatabaseResultSet& value_;
};

template <>
class ParamCast<IDatabaseResultSet*> {
public:
    ParamCast(AMX* amx, cell* params, int idx) noexcept
    {
        value_ = ParamLookup<IDatabaseResultSet>::Val(params[idx]);
    }

    ~ParamCast()
    {
    }

    ParamCast(ParamCast<IDatabaseResultSet*> const&) = delete;
    ParamCast(ParamCast<IDatabaseResultSet*>&&) = delete;

    operator IDatabaseResultSet*()
    {
        return value_;
    }

    static constexpr int Size = 1;

private:
    IDatabaseResultSet* value_;
};

template <>
class ParamCast<PawnScript&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
        : value_(PawnManager::Get()->amxToScript_.find(amx)->second)
    {
    }

    ~ParamCast()
    {
    }

    ParamCast(ParamCast<IDatabaseConnection*> const&) = delete;
    ParamCast(ParamCast<IDatabaseConnection*>&&) = delete;

    operator PawnScript&()
    {
        return *value_;
    }

    static constexpr int Size = 0;

private:
    PawnScript* value_;
};

// Disable the ref version.
template <>
class ParamCast<Vector3 const&> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector3*> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector3 const*> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector3> {
public:
    ParamCast([[maybe_unused]] AMX* amx, cell* params, int idx)
        : value_ { amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]) }
    {
    }

    ~ParamCast()
    {
    }

    operator Vector3() const
    {
        return value_;
    }

    static constexpr int Size = 3;

    using type = Vector3;

private:
    Vector3
        value_;
};

template <>
class ParamCast<Vector3&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
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

    operator Vector3&()
    {
        return value_;
    }

    static constexpr int Size = 3;

    using type = Vector3&;

private:
    Vector3
        value_;

    cell
        *x_,
        *y_,
        *z_;
};

template <>
class ParamCast<Vector2 const&> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector2*> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector2> {
public:
    ParamCast([[maybe_unused]] AMX* amx, cell* params, int idx)
        : value_ { amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]) }
    {
    }

    ~ParamCast()
    {
    }

    operator Vector2() const
    {
        return value_;
    }

    static constexpr int Size = 2;

    using type = Vector2;

private:
    Vector2
        value_;
};

template <>
class ParamCast<Vector2&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
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

    operator Vector2&()
    {
        return value_;
    }

    static constexpr int Size = 2;

    using type = Vector2&;

private:
    Vector2
        value_;

    cell
        *x_,
        *y_;
};

template <>
class ParamCast<Vector4 const&> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector4*> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<Vector4> {
public:
    ParamCast([[maybe_unused]] AMX* amx, cell* params, int idx)
        : value_ { amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]), amx_ctof(params[idx + 3]) }
    {
    }

    ~ParamCast()
    {
    }

    operator Vector4() const
    {
        return value_;
    }

    static constexpr int Size = 4;

    using type = Vector4;

private:
    Vector4
        value_;
};

template <>
class ParamCast<Vector4&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
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

    operator Vector4&()
    {
        return value_;
    }

    static constexpr int Size = 4;

    using type = Vector4&;

private:
    Vector4
        value_;

    cell
        *x_,
        *y_,
        *z_,
        *w_;
};

template <>
class ParamCast<GTAQuat const&> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<GTAQuat*> {
public:
    ParamCast(AMX*, cell*, int) = delete;
    ParamCast() = delete;
};

template <>
class ParamCast<GTAQuat> {
public:
    ParamCast([[maybe_unused]] AMX* amx, cell* params, int idx)
        : value_(amx_ctof(params[idx + 0]), amx_ctof(params[idx + 1]), amx_ctof(params[idx + 2]), amx_ctof(params[idx + 3]))
    {
    }

    ~ParamCast()
    {
    }

    operator GTAQuat() const
    {
        return value_;
    }

    static constexpr int Size = 4;

    using type = GTAQuat;

private:
    GTAQuat
        value_;
};

template <>
class ParamCast<GTAQuat&> {
public:
    ParamCast(AMX* amx, cell* params, int idx)
    {
        amx_GetAddr(amx, params[idx + 0], &w_);
        amx_GetAddr(amx, params[idx + 1], &x_);
        amx_GetAddr(amx, params[idx + 2], &y_);
        amx_GetAddr(amx, params[idx + 3], &z_);
        value_.q.w = amx_ctof(*w_);
        value_.q.x = amx_ctof(*x_);
        value_.q.y = amx_ctof(*y_);
        value_.q.z = amx_ctof(*z_);
    }

    ~ParamCast()
    {
        // Write the value back in to memory.
        *w_ = amx_ftoc(value_.q.w);
        *x_ = amx_ftoc(value_.q.x);
        *y_ = amx_ftoc(value_.q.y);
        *z_ = amx_ftoc(value_.q.z);
    }

    operator GTAQuat&()
    {
        return value_;
    }

    static constexpr int Size = 4;

    using type = GTAQuat&;

private:
    GTAQuat
        value_;

    cell
        *w_,
        *x_,
        *y_,
        *z_;
};

class NotImplemented : public std::logic_error {
public:
    NotImplemented()
        : std::logic_error { "Pawn native not yet implemented." }
    {
    }
};
}
