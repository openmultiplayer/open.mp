// This class maps one set of IDs to another set.  For example the gang zones internal pool is
// infinite, but the external Pawn API has two separate finite pools - a global pool and a per-
// player pool (technically that's `MAX_PLAYERS + 1` pools, not two).  The SDK can simply create as
// many gang zones as desired and assign them ever increasing IDs.

/// `X` (`MAX`) is the highest valid legacy ID PLUS ONE.
/// `N` (`MIN`) is the lowest valid legacy ID.
/// `I` (`INVALID`) is the invalid legacy ID returned in errors.
/// `F` (`FAIL`) is the invalid new ID returned in errors.
template <int /*MA*/ X, int /*MI*/ N = 0, int I /*NVALID*/ = -1, int F /*AIL*/ = 0>
struct ILegacyIDMapper
{
	static const int MIN;
	static const int MAX;
	static const int INVALID;
	static const int NOT_FOUND;

	/// Request a new legacy ID.
	virtual int reserve() = 0;

	/// Store the given new ID in a legacy ID.
	virtual void set(int legacy, int real) = 0;

	/// Release a previously used legacy ID and return the new ID it referenced.
	virtual void release(int legacy) = 0;

	/// Get the legacy ID for the given new ID, or `INVALID`.
	virtual int toLegacy(int real) const = 0;

	/// Get the new ID for the given legacy ID, or `NOT_FOUND`.
	virtual int fromLegacy(int legacy) const = 0;
};

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int ILegacyIDMapper<X, N, I, F>::MIN = N;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int ILegacyIDMapper<X, N, I, F>::MAX = X;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int ILegacyIDMapper<X, N, I, F>::INVALID = I;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int ILegacyIDMapper<X, N, I, F>::NOT_FOUND = F;

// TODO: Use a faster `toLegacy` lookup system.  Maybe binary search or similar.
template <int /*MA*/ X, int /*MI*/ N = 0, int I /*NVALID*/ = -1, int F /*AIL*/ = 0>
class FiniteLegacyIDMapper final : public ILegacyIDMapper<X, N, I, F>
{
public:
	static const int MIN;
	static const int MAX;
	static const int INVALID;
	static const int NOT_FOUND;

private:
	StaticArray<int, MAX - MIN> ids_;

public:
	FiniteLegacyIDMapper()
		: ids_()
	{
		ids_.fill(NOT_FOUND);
	}

	/// Request a new legacy ID.
	virtual int reserve() override
	{
		for (size_t legacy = 0; legacy != MAX - MIN; ++legacy)
		{
			if (ids_[legacy] == NOT_FOUND)
			{
				return legacy + MIN;
			}
		}
		return INVALID;
	}

	/// Store the given new ID in a legacy ID.
	virtual void set(int legacy, int real) override
	{
		if (legacy >= MIN && legacy < MAX)
		{
			ids_[legacy - MIN] = real;
		}
	}

	/// Release a previously used legacy ID.
	virtual void release(int legacy) override
	{
		if (legacy >= MIN && legacy < MAX)
		{
			ids_[legacy - MIN] = NOT_FOUND;
		}
	}

	/// Get the legacy ID for the given new ID, or `INVALID`.
	virtual int toLegacy(int real) const override
	{
		for (size_t legacy = 0; legacy != MAX - MIN; ++legacy)
		{
			if (ids_[legacy] == real)
			{
				return legacy + MIN;
			}
		}
		return INVALID;
	}

	/// Get the new ID for the given legacy ID, or `NOT_FOUND`.
	virtual int fromLegacy(int legacy) const override
	{
		if (legacy < MIN || legacy >= MAX)
		{
			return NOT_FOUND;
		}
		return ids_[legacy - MIN];
	}
};

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int FiniteLegacyIDMapper<X, N, I, F>::MIN = N;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int FiniteLegacyIDMapper<X, N, I, F>::MAX = X;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int FiniteLegacyIDMapper<X, N, I, F>::INVALID = I;

template <int /*MA*/ X, int /*MI*/ N, int I /*NVALID*/, int F /*AIL*/>
const int FiniteLegacyIDMapper<X, N, I, F>::NOT_FOUND = F;
