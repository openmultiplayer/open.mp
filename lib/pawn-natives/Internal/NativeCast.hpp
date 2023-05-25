#pragma once

#include <stdexcept>
#include <string>
#include <memory>
#include <optional>
#include <amx/amx.h>

namespace pawn_natives
{
// This is for any casts that can't go on, but where this is somewhat expected.  For example, a
// cast to a player when there is no player.
class ParamCastFailure : public std::invalid_argument
{
public:
	explicit ParamCastFailure()
		: std::invalid_argument("ParamCast failed acceptably.")
	{
	}
};

// This is for true cast errors.
class ParamCastError : public std::invalid_argument
{
public:
	explicit ParamCastError()
		: std::invalid_argument("ParamCast had an exception.")
	{
	}
};

template <typename T, typename = void>
struct ParamLookup
{
	static T Val(cell ref)
	{
		return (T)ref;
	}

	static T* Ptr(cell* ref)
	{
		return (T*)ref;
	}
};

template <>
struct ParamLookup<float>
{
	static float Val(cell ref)
	{
		return amx_ctof(ref);
	}

	static float* Ptr(cell* ref)
	{
		return (float*)ref;
	}
};

template <typename T>
class ParamCast
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
		: value_(ParamLookup<T>::Val(params[idx]))
	{
	}

	ParamCast(ParamCast<T> const&) = delete;
	ParamCast(ParamCast<T>&&) = delete;

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
	}

	operator T()
	{
		return value_;
	}

	static constexpr int Size = 1;

private:
	T
		value_;
};

template <typename T>
class ParamCast<T const>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
		: value_(ParamLookup<T>::Val(params[idx]))
	{
		// In theory, because `T` could contain `const`, we don't actually
		// need specialisations for constant parameters.  The pointer would
		// point in to AMX memory but be `const`.  However, people can cast
		// away `const` if they REALLY want to, so make a copy of the
		// parameter so that they still can't do that to modify the
		// original.  If they REALLY REALLY want to modify the original
		// parameter in AMX memory they will have to re-extract the pointer
		// from `params`.
	}

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
	}

	ParamCast(ParamCast<T const> const&) = delete;
	ParamCast(ParamCast<T const>&&) = delete;

	operator T const() const
	{
		return value_;
	}

	static constexpr int Size = 1;

private:
	T
		value_;
};

template <typename T>
class ParamCast<T*>
{
public:
	ParamCast(AMX*, cell*, int) = delete;
	ParamCast() = delete;
};

template <typename T>
class ParamCast<T const*>
{
public:
	ParamCast(AMX*, cell*, int) = delete;
	ParamCast() = delete;
};

template <typename T>
class ParamCast<T&>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
	{
		cell*
			src;
		amx_GetAddr(amx, params[idx], &src);
		if (src == nullptr)
		{
			throw pawn_natives::ParamCastFailure();
		}
		value_ = ParamLookup<T>::Ptr(src);
	}

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
		// This one doesn't because we are passing the direct pointer, which means any writes
		// are done directly in to AMX memory.
	}

	ParamCast(ParamCast<T&> const&) = delete;
	ParamCast(ParamCast<T&>&&) = delete;

	operator T&()
	{
		return *value_;
	}

	static constexpr int Size = 1;

private:
	T*
		value_;
};

template <typename T>
class ParamCast<T const&>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
	{
		// In theory, because `T` could contain `const`, we don't actually
		// need specialisations for constant parameters.  The pointer would
		// point in to AMX memory but be `const`.  However, people can cast
		// away `const` if they REALLY want to, so make a copy of the
		// parameter so that they still can't do that to modify the
		// original.  If they REALLY REALLY want to modify the original
		// parameter in AMX memory they will have to re-extract the pointer
		// from `params`.
		cell*
			src;
		amx_GetAddr(amx, params[idx], &src);
		if (src == nullptr)
		{
			throw pawn_natives::ParamCastFailure();
		}
		value_ = ParamLookup<T>::Ptr(src);
	}

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
	}

	ParamCast(ParamCast<T const&> const&) = delete;
	ParamCast(ParamCast<T const&>&&) = delete;

	operator T const &() const
	{
		return *value_;
	}

	static constexpr int Size = 1;

private:
	T*
		value_;
};

// Use `string &`.
template <>
class ParamCast<char*>
{
public:
	ParamCast(AMX* amx, cell* params, int idx) = delete;
	ParamCast(ParamCast<char*> const&) = delete;
	ParamCast(ParamCast<char*>&&) = delete;
};

// Use `string const &`.
template <>
class ParamCast<char const*>
{
public:
	ParamCast(AMX* amx, cell* params, int idx) = delete;
	ParamCast(ParamCast<char const*> const&) = delete;
	ParamCast(ParamCast<char const*>&&) = delete;
};

// `string *` doesn't exist any more.  If it is a return value, the convention
// is to use `string &`.  If it is an input, use `string const &`.
template <>
class ParamCast<std::string*>
{
public:
	ParamCast(AMX* amx, cell* params, int idx) = delete;
	ParamCast(ParamCast<std::string*> const&) = delete;
	ParamCast(ParamCast<std::string*>&&) = delete;
};

template <>
class ParamCast<std::string&>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
		: len_((int)params[idx + 1])
	{
		// Can't use `amx_StrParam` here, it allocates on the stack.  This
		// code wraps a lot of `sampgdk`, which fortunately is entirely
		// const-correct so we don't need to worry about strings not being
		// copied incorrectly.  We can also make the assumption that any
		// string is immediately followed by its length when it is an
		// output.
		if (len_ < 0)
			throw std::length_error("Invalid string length.");
		if (len_)
		{
			amx_GetAddr(amx, params[idx], &addr_);
			if (addr_ == nullptr)
			{
				throw pawn_natives::ParamCastFailure();
			}
#ifdef _WIN32
			char* src = (char*)_malloca(len_);
			amx_GetString(src, addr_, 0, len_);
			value_ = src;
			_freea(src);
#else
			char* src = (char*)alloca(len_);
			amx_GetString(src, addr_, 0, len_);
			value_ = src;
#endif
		}
		else
		{
			value_.clear();
			addr_ = nullptr;
		}
	}

	~ParamCast()
	{
		// This is the only version that actually needs to write data back.
		if (addr_)
			amx_SetStringLen(addr_, value_.data(), value_.length(), 0, 0, len_);
	}

	ParamCast(ParamCast<std::string&> const&) = delete;
	ParamCast(ParamCast<std::string&>&&) = delete;

	operator std::string&()
	{
		return value_;
	}

	static constexpr int Size = 2;

private:
	int
		len_;

	cell*
		addr_;

	std::string
		value_;
};

template <>
class ParamCast<std::string const&>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
	{
		// Can't use `amx_StrParam` here, it allocates on the stack.  This
		// `const` version is not optional at all - it ensures that the
		// string data is NOT written back.
		cell*
			addr;
		int
			len;
		amx_GetAddr(amx, params[idx], &addr);
		if (addr == nullptr)
		{
			throw pawn_natives::ParamCastFailure();
		}
		amx_StrLen(addr, &len);
		if (len > 0)
		{
#ifdef _WIN32
			char* src = (char*)_malloca(len + 1);
			amx_GetString(src, addr, 0, len + 1);
			value_ = src;
			_freea(src);
#else
			char* src = (char*)alloca(len + 1);
			amx_GetString(src, addr, 0, len + 1);
			value_ = src;
#endif
		}
		else
			value_.clear();
	}

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
	}

	ParamCast(ParamCast<std::string const&> const&) = delete;
	ParamCast(ParamCast<std::string const&>&&) = delete;

	operator std::string const &()
	{
		return value_;
	}

	static constexpr int Size = 1;

private:
	std::string
		value_;
};

template <>
class ParamCast<cell const*>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
	{
		cell* cptr;
		amx_GetAddr(amx, params[idx], &cptr);
		if (cptr == nullptr)
		{
			throw pawn_natives::ParamCastFailure();
		}
		value_ = cptr;
	}

	~ParamCast()
	{
		// Some versions may need to write data back here, but not this one.
	}

	ParamCast(ParamCast<std::string const&> const&) = delete;
	ParamCast(ParamCast<std::string const&>&&) = delete;

	operator cell const *()
	{
		return value_;
	}

	static constexpr int Size = 1;

private:
	cell const*
		value_;
};

template <size_t N, typename... TS>
struct ParamArray
{
};

template <size_t N, typename T, typename... TS>
struct ParamArray<N, T, TS...>
{
	template <class F, typename... NS>
	static inline auto Call(F that, AMX* amx, cell* params, size_t prev, NS&&... vs)
		-> decltype(ParamArray<N - 1, TS...>::Call(that, amx, params, prev + ParamCast<T>::Size, std::forward<NS>(vs)..., ParamCast<T>(amx, params, prev)))
	{
		return ParamArray<N - 1, TS...>::Call(that, amx, params, prev + ParamCast<T>::Size, std::forward<NS>(vs)..., ParamCast<T>(amx, params, prev));
	}
};

template <>
struct ParamArray<0>
{
	template <class F, typename... NS>
	static inline auto Call(F that, AMX* amx, cell* params, size_t prev, NS&&... vs)
		-> decltype(that->Do(std::forward<NS>(vs)...))
	{
		return that->Do(std::forward<NS>(vs)...);
	}
};

template <typename... TS>
struct ParamData
{
};

template <typename T, typename... TS>
struct ParamData<T, TS...>
{
	static inline constexpr int Sum()
	{
		return ParamCast<T>::Size + ParamData<TS...>::Sum();
		//return 0 + ... + ParamCast<TS>::Size;
	}

	template <class F>
	static inline auto Call(F that, AMX* amx, cell* params)
		-> decltype(ParamArray<sizeof...(TS) + 1, T, TS...>::Call(that, amx, params, 1))
	{
		return ParamArray<sizeof...(TS) + 1, T, TS...>::Call(that, amx, params, 1);
	}
};

template <>
struct ParamData<>
{
	static inline constexpr int Sum()
	{
		return 0;
	}

	template <class F>
	static inline auto Call(F that, AMX*, cell*)
		-> decltype(that->Do())
	{
		return that->Do();
	}
};
}
