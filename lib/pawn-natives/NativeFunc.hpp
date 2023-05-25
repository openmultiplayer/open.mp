#pragma once

#include <list>
#include <stdexcept>

#include "Internal/NativeCast.hpp"
#include "NativeImport.hpp"

#define PAWN_NATIVES_HAS_FUNC

namespace pawn_natives
{
int AmxLoad(AMX* amx);

class NativeFuncBase
{
public:
	void Init()
	{
		// Install this as a native.
		//Install(NULL);
	}

protected:
	NativeFuncBase(unsigned int count, char const* const name, AMX_NATIVE native)
		: count_(count * sizeof(cell))
		, name_(name)
		, native_(native)
		, amx_(0)
		, params_(0)
	{
		if (!all_)
			all_ = new std::list<NativeFuncBase*>();
		if (all_)
			all_->push_back(this);
	}

	~NativeFuncBase() = default;

	AMX* GetAMX() const { return amx_; }
	cell* GetParams() const { return params_; }
	static constexpr cell no_args[] = { 0 };

	template <cell FailRet>
	cell CallDoOuter(AMX* amx, cell* params)
	{
		cell
			ret
			= FailRet;
		if (amx)
		{
			// Check that there are enough parameters.
			amx_ = amx;

			if (params)
				params_ = params;
			else
			{
				// NULL params means zero params, at least for GDK
				params_ = (cell*)(&no_args);
			}

			try
			{
				if (count_ > (unsigned int)params_[0])
					throw std::invalid_argument("Insufficient arguments.");
				ret = this->CallDoInner(amx, params);
			}
			catch (ParamCastFailure const&)
			{
				// Acceptable failure (lookup failed etc.)
			}
			catch (std::exception const& e)
			{
				char
					msg[1024];
				sprintf(msg, "Exception in %s: \"%s\"", name_, e.what());
				LOG_NATIVE_ERROR(msg);
			}
			catch (...)
			{
				char
					msg[1024];
				sprintf(msg, "Unknown exception in in %s", name_);
				LOG_NATIVE_ERROR(msg);
				params_ = 0;
				amx_ = 0;
				throw;
			}
			params_ = 0;
			amx_ = 0;
		}
		return (cell)ret;
	}

private:
	virtual cell CallDoInner(AMX*, cell*) = 0;

	friend int AmxLoad(AMX* amx);

	NativeFuncBase() = delete;
	NativeFuncBase(NativeFuncBase const&) = delete;
	NativeFuncBase(NativeFuncBase const&&) = delete;
	NativeFuncBase const& operator=(NativeFuncBase const&) const = delete;
	NativeFuncBase const& operator=(NativeFuncBase const&&) const = delete;

	unsigned int
		count_;

	char const* const
		name_;

	AMX_NATIVE const
		native_;

	AMX*
		amx_;

	cell*
		params_;

	static std::list<NativeFuncBase*>*
		all_;
};

template <typename RET, typename... TS>
class NativeFunc : protected NativeFuncBase
{
public:
	inline RET operator()(TS&&... args)
	{
		return Do(std::forward<TS>(args)...);
	}

	virtual RET Do(TS...) const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(ParamData<TS...>::Sum(), name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		RET
			ret
			= ParamData<TS...>::Call(this, amx, params);
		// TODO: static_assert that `sizeof (RET) == sizeof (cell)`.
		return *(cell*)&ret;
	}
};

template <typename... TS>
class NativeFunc<void, TS...> : protected NativeFuncBase
{
public:
	inline void operator()(TS&&... args)
	{
		Do(std::forward<TS>(args)...);
	}

	virtual void Do(TS...) const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(ParamData<TS...>::Sum(), name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		ParamData<TS...>::Call(this, amx, params);
		return 1;
	}
};

template <typename... TS>
class NativeFunc<bool, TS...> : protected NativeFuncBase
{
public:
	inline bool operator()(TS... args)
	{
		return Do(args...);
	}

	virtual bool Do(TS...) const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(ParamData<TS...>::Sum(), name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		return ParamData<TS...>::Call(this, amx, params) ? 1 : 0;
	}
};

template <typename RET>
class NativeFunc<RET> : protected NativeFuncBase
{
public:
	inline RET operator()()
	{
		return Do();
	}

	virtual RET Do() const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(0, name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		RET
			ret
			= ParamData<>::Call(this, amx, params);
		// TODO: static_assert that `sizeof (RET) == sizeof (cell)`.
		return *(cell*)&ret;
	}
};

template <>
class NativeFunc<void> : protected NativeFuncBase
{
public:
	inline void operator()()
	{
		Do();
	}

	virtual void Do() const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(0, name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		ParamData<>::Call(this, amx, params);
		return 1;
	}
};

template <>
class NativeFunc<bool> : protected NativeFuncBase
{
public:
	inline bool operator()()
	{
		return Do();
	}

	virtual bool Do() const = 0;

protected:
	NativeFunc(char const* const name, AMX_NATIVE native)
		: NativeFuncBase(0, name, native)
	{
	}
	~NativeFunc() = default;

private:
	cell CallDoInner(AMX* amx, cell* params)
	{
		return ParamData<>::Call(this, amx, params) ? 1 : 0;
	}
};
}

// The hooks and calls for each class are always static, because otherwise it
// would make installing hooks MUCH harder - we would need stubs that could
// handle class pointers.  Doing that would negate needing a different class for
// every hook type, even when the parameters are the same, but this way is
// probably not much more generated code, and vastly simpler.
//
// The inheritance from `NativeFuncBase` is protected, because we don't want
// normal users getting in to that data.  However, we do want them to be able to
// use the common `IsEnabled` method, so re-export it.
#define PAWN_NATIVE_DECL(used_namespace, func, type) PAWN_NATIVE_DECL_(used_namespace, failret, func, type)

#define PAWN_NATIVE_DECL_(used_namespace, failret, func, params)                     \
	template <typename F>                                                            \
	class Native_##func##_                                                           \
	{                                                                                \
	};                                                                               \
                                                                                     \
	template <typename RET, typename... TS>                                          \
	class Native_##func##_<RET(TS...)> : public pawn_natives::NativeFunc<RET, TS...> \
	{                                                                                \
		constexpr static const decltype(failret) FailRet = failret;                  \
                                                                                     \
	public:                                                                          \
		Native_##func##_();                                                          \
                                                                                     \
		RET Do(TS...) const override;                                                \
                                                                                     \
	private:                                                                         \
		static cell AMX_NATIVE_CALL Call(AMX* amx, cell* args);                      \
		using Base = pawn_natives::NativeFunc<RET, TS...>;                           \
	};                                                                               \
                                                                                     \
	template class Native_##func##_<params>;                                         \
	using Native_##func = Native_##func##_<params>;                                  \
                                                                                     \
	namespace used_namespace                                                         \
	{                                                                                \
		extern Native_##func func;                                                   \
	}

// We can't pass exceptions to another module easily, so just don't...

#define PAWN_NATIVE_DEFN(used_namespace, func, params) PAWN_NATIVE_DEFN_(used_namespace, 0, func, params)
#define PAWN_NATIVE_DEFN_FAILRET(used_namespace, failret, func, params) PAWN_NATIVE_DEFN_(used_namespace, failret, func, params)

#define PAWN_NATIVE_DEFN_(used_namespace, failret, func, params)               \
                                                                               \
	template <>                                                                \
	cell AMX_NATIVE_CALL Native_##func::Call(AMX* amx, cell* args)             \
	{                                                                          \
		return used_namespace::func.CallDoOuter<failret>(amx, args);           \
	}                                                                          \
                                                                               \
	template <>                                                                \
	Native_##func::Native_##func##_()                                          \
		: Base(#func, (AMX_NATIVE)&Call)                                       \
	{                                                                          \
	}                                                                          \
                                                                               \
	Native_##func used_namespace::func;                                        \
                                                                               \
	template <>                                                                \
	PAWN_NATIVE__RETURN(params)                                                \
	Native_##func::                                                            \
		Do(PAWN_NATIVE__PARAMETERS(params)) const;                             \
                                                                               \
	template <typename RET, typename... TS>                                    \
	typename pawn_natives::ReturnResolver<RET>::type NATIVE_##func(TS... args) \
	{                                                                          \
		try                                                                    \
		{                                                                      \
			PAWN_NATIVE__GET_RETURN(params)                                    \
			(used_namespace::func.Do(args...));                                \
		}                                                                      \
		catch (std::exception & e)                                             \
		{                                                                      \
			char msg[1024];                                                    \
			sprintf(msg, "Exception in _" #func ": \"%s\"", e.what());         \
			LOG_NATIVE_ERROR(msg);                                             \
		}                                                                      \
		catch (...)                                                            \
		{                                                                      \
			LOG_NATIVE_ERROR("Unknown exception in _" #func);                  \
		}                                                                      \
		PAWN_NATIVE__DEFAULT_RETURN(params);                                   \
	}                                                                          \
                                                                               \
	template <>                                                                \
	PAWN_NATIVE__RETURN(params)                                                \
	Native_##func::                                                            \
		Do(PAWN_NATIVE__PARAMETERS(params)) const

#define PAWN_NATIVE_DECLARE PAWN_NATIVE_DECL
#define PAWN_NATIVE_DEFINE PAWN_NATIVE_DEFN
#define PAWN_NATIVE_DEFINE_FAILRET PAWN_NATIVE_DEFN_FAILRET

#define PAWN_NATIVE(used_namespace, func, params)          \
	PAWN_NATIVE_DECL_(used_namespace, false, func, params) \
	PAWN_NATIVE_DEFN_(used_namespace, false, func, params)

#define PAWN_NATIVE_FAILRET(used_namespace, failret, func, params) \
	PAWN_NATIVE_DECL_(used_namespace, failret, func, params)       \
	PAWN_NATIVE_DEFN_(used_namespace, failret, func, params)

#if 0

// Example:

// In your header:
PAWN_NATIVE_DECL(SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a));

// In your code:
PAWN_NATIVE_DEFN(SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a))
{
	// Implementation here...
	SetPlayerPos(playerid, x, y, z);
	return SetPlayerFacingAngle(playerid, a);
}

#endif
