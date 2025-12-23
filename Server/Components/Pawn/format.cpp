// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#include "format.hpp"
#include "Manager/Manager.hpp"

#ifndef WIN32
#include <math.h>
#endif

// Adapted from Quake3's vsprintf
// Thanks to cybermind for linking me to this :)
// I made the following changes:
// 1. Fixed spacing to be AMX Mod X standard
// 2. Added 'n' support, no buffer overflows
// 3. Templatized input/output buffers

#define ALT 0x00000001 /* alternate form */
#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
#define LADJUST 0x00000004 /* left adjustment */
#define LONGDBL 0x00000008 /* long double */
#define LONGINT 0x00000010 /* long integer */
#define QUADINT 0x00000020 /* quad integer */
#define SHORTINT 0x00000040 /* short integer */
#define ZEROPAD 0x00000080 /* zero (as opposed to blank) pad */
#define FPT 0x00000100 /* floating point number */
#define UPPERDIGITS 0x00000200 /* make alpha digits uppercase */
#define to_digit(c) ((c) - '0')
#define is_digit(c) ((unsigned)to_digit(c) <= 9)
#define to_char(n) ((n) + '0')
#define CHECK_ARGS(n)                                                                                                                                  \
	if ((arg + n) > args)                                                                                                                              \
	{                                                                                                                                                  \
		char* formatStr;                                                                                                                               \
		int formatLength;                                                                                                                              \
                                                                                                                                                       \
		amx_StrLen(format, &formatLength);                                                                                                             \
		if (formatLength > 0 && (formatStr = reinterpret_cast<char*>(alloca(formatLength + 1))))                                                       \
		{                                                                                                                                              \
			amx_GetString(formatStr, format, false, formatLength + 1);                                                                                 \
		}                                                                                                                                              \
		else                                                                                                                                           \
		{                                                                                                                                              \
			formatStr = const_cast<char*>("");                                                                                                         \
		}                                                                                                                                              \
		PawnManager::Get()->core->logLn(LogLevel::Error, "String formatted incorrectly - parameter: %d, total: %d, format: %s", arg, args, formatStr); \
		return 0;                                                                                                                                      \
	}

template size_t atcprintf<cell, cell>(cell*, size_t, const cell*, AMX*, const cell*, int*);
template size_t atcprintf<char, cell>(char*, size_t, const cell*, AMX*, const cell*, int*);

template <typename U, typename S>
void AddString(U** buf_p, size_t& maxlen, const S* string, int width, int prec, int flags)
{
	int size = 0;
	U* buf;
	static S nlstr[] = { '(', 'n', 'u', 'l', 'l', ')', '\0' };
	if (*string > UNPACKEDMAX)
	{
		buf = *buf_p;

		if (string == NULL)
		{
			string = nlstr;
			prec = -1;
		}

		if (prec >= 0)
		{
			for (size = 0; size < prec; size++)
			{
				if (((char*)string)[size ^ (sizeof(cell) - 1)] == '\0')
					break;
			}
		}
		else
		{
			while (((char*)string)[size ^ (sizeof(cell) - 1)])
				++size;
		}

		if (size > (int)maxlen)
		{
			size = maxlen;
		}

		if ((flags & LADJUST))
		{
			while ((size < width) && maxlen)
			{
				*buf++ = (flags & ZEROPAD) ? '0' : ' ';
				width--;
				maxlen--;
			}
		}

		maxlen -= size;
		width -= size;

		while (size--)
		{
			*buf++ = static_cast<U>(*(char*)((intptr_t)string ^ (sizeof(cell) - 1)));
			string = (S*)((char*)string + 1);
		}

		// left justify if required.  backwards from most specifiers.
		if ((flags & LADJUST) == 0)
		{
			while (width-- > 0 && maxlen)
			{
				// right-padding only with spaces, ZEROPAD is ignored
				*buf++ = ' ';
				maxlen--;
			}
		}

		*buf_p = buf;
	}
	else
	{
		buf = *buf_p;

		if (string == NULL)
		{
			string = nlstr;
			prec = -1;
		}

		if (prec >= 0)
		{
			for (size = 0; size < prec; size++)
			{
				if (string[size] == '\0')
					break;
			}
		}
		else
		{
			while (string[size++])
				;
			size--;
		}

		if (size > (int)maxlen)
		{
			size = maxlen;
		}

		if ((flags & LADJUST))
		{
			while ((size < width) && maxlen)
			{
				*buf++ = (flags & ZEROPAD) ? '0' : ' ';
				width--;
				maxlen--;
			}
		}

		maxlen -= size;
		width -= size;

		while (size--)
		{
			*buf++ = static_cast<U>(*string++);
		}

		// left justify if required.  backwards from most specifiers.
		if ((flags & LADJUST) == 0)
		{
			while (width-- > 0 && maxlen)
			{
				// right-padding only with spaces, ZEROPAD is ignored
				*buf++ = ' ';
				maxlen--;
			}
		}

		*buf_p = buf;
	}
}

template <typename U>
void AddFloat(U** buf_p, size_t& maxlen, double fval, int width, int prec, int flags)
{

	if (maxlen < 3)
	{
		return;
	}

	int digits; // non-fraction part digits
	double tmp; // temporary
	U* buf = *buf_p; // output buffer pointer
	int val; // temporary
	int sign = 0; // 0: positive, 1: negative
	int fieldlength; // for padding
	int significant_digits = 0; // number of significant digits written
	const int MAX_SIGNIFICANT_DIGITS = 16;

	if (isnan(fval))
	{
		// NaN
		*(buf++) = 'N';
		*(buf++) = 'a';
		*(buf++) = 'N';
		maxlen -= 3;
	}
	else
	{
		// default precision
		if (prec < 0)
		{
			prec = 6;
		}

		// get the sign
		if (fval < 0)
		{
			fval = -fval;
			sign = 1;
		}

		// compute whole-part digits count
		digits = (int)log10(fval) + 1;

		// Only print 0.something if 0 < fval < 1
		if (digits < 1)
		{
			digits = 1;
		}

		// compute the field length
		fieldlength = digits + prec + ((prec > 0) ? 1 : 0) + sign;

		// minus sign BEFORE left padding if padding with zeros
		if (sign && maxlen && (flags & ZEROPAD))
		{
			*buf++ = '-';
			maxlen--;
		}

		// right justify if required
		if ((flags & LADJUST) == 0)
		{
			while ((fieldlength < width) && maxlen)
			{
				*buf++ = (flags & ZEROPAD) ? '0' : ' ';
				width--;
				maxlen--;
			}
		}

		// minus sign AFTER left padding if padding with spaces
		if (sign && maxlen && !(flags & ZEROPAD))
		{
			*buf++ = '-';
			maxlen--;
		}

		// write the whole part
		tmp = pow(10.0, digits - 1);
		while ((digits--) && maxlen)
		{
			if (++significant_digits > MAX_SIGNIFICANT_DIGITS)
			{
				*buf++ = '0';
			}
			else
			{
				val = (int)(fval / tmp);
				*buf++ = '0' + val;
				fval -= val * tmp;
				tmp *= 0.1;
			}
			maxlen--;
		}

		// write the fraction part
		if (maxlen && prec)
		{
			*buf++ = '.';
			maxlen--;
		}

		tmp = pow(10.0, prec);

		fval *= tmp;
		while (prec-- && maxlen)
		{
			if (++significant_digits > MAX_SIGNIFICANT_DIGITS)
			{
				*buf++ = '0';
			}
			else
			{
				tmp *= 0.1;
				val = (int)(fval / tmp);
				*buf++ = '0' + val;
				fval -= val * tmp;
			}
			maxlen--;
		}

		// left justify if required
		if (flags & LADJUST)
		{
			while ((fieldlength < width) && maxlen)
			{
				// right-padding only with spaces, ZEROPAD is ignored
				*buf++ = ' ';
				width--;
				maxlen--;
			}
		}
	}

	// update parent's buffer pointer
	*buf_p = buf;
}

template <typename U>
void AddBinary(U** buf_p, size_t& maxlen, unsigned int val, int width, int flags)
{
	char text[32];
	int digits;
	U* buf;

	digits = 0;
	do
	{
		if (val & 1)
		{
			text[digits++] = '1';
		}
		else
		{
			text[digits++] = '0';
		}
		val >>= 1;
	} while (val);

	buf = *buf_p;

	if (!(flags & LADJUST))
	{
		while (digits < width && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			width--;
			maxlen--;
		}
	}

	while (digits-- && maxlen)
	{
		*buf++ = text[digits];
		width--;
		maxlen--;
	}

	if (flags & LADJUST)
	{
		while (width-- && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			maxlen--;
		}
	}

	*buf_p = buf;
}

template <typename U>
void AddUInt(U** buf_p, size_t& maxlen, unsigned int val, int width, int flags)
{
	U text[32];
	int digits = 0;
	U* buf = *buf_p;

	do
	{
		text[digits++] = '0' + val % 10;
		val /= 10;
	} while (val);

	if (!(flags & LADJUST))
	{
		while (digits < width && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			width--;
			maxlen--;
		}
	}

	while (digits-- && maxlen)
	{
		*buf++ = text[digits];
		width--;
		maxlen--;
	}

	if (flags & LADJUST)
	{
		while (width-- && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			maxlen--;
		}
	}

	*buf_p = buf;
}

template <typename U>
void AddInt(U** buf_p, size_t& maxlen, int val, int width, int flags)
{
	U text[32];
	int digits = 0;
	int signedVal = val;
	U* buf = *buf_p;
	unsigned int unsignedVal = abs(val);

	do
	{
		text[digits++] = '0' + unsignedVal % 10;
		unsignedVal /= 10;
	} while (unsignedVal);

	if (signedVal < 0)
	{
		if (flags & ZEROPAD)
		{
			*buf++ = '-';
			maxlen--;
		}
		else
		{
			text[digits++] = '-';
		}
	}

	if (!(flags & LADJUST))
	{
		while (digits < width && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			width--;
			maxlen--;
		}
	}

	while (digits-- && maxlen)
	{
		*buf++ = text[digits];
		width--;
		maxlen--;
	}

	if (flags & LADJUST)
	{
		while (width-- && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			maxlen--;
		}
	}

	*buf_p = buf;
}

template <typename U>
void AddHex(U** buf_p, size_t& maxlen, unsigned int val, int width, int flags)
{
	U text[32];
	int digits = 0;
	U* buf = *buf_p;
	U digit = 0;
	int hexadjust = 0;

	if (flags & UPPERDIGITS)
	{
		hexadjust = 'A' - '9' - 1;
	}
	else
	{
		hexadjust = 'a' - '9' - 1;
	}

	do
	{
		digit = ('0' + val % 16);
		if (digit > '9')
		{
			digit += hexadjust;
		}

		text[digits++] = digit;
		val /= 16;
	} while (val);

	if (!(flags & LADJUST))
	{
		while (digits < width && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			width--;
			maxlen--;
		}
	}

	while (digits-- && maxlen)
	{
		*buf++ = text[digits];
		width--;
		maxlen--;
	}

	if (flags & LADJUST)
	{
		while (width-- && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			maxlen--;
		}
	}

	*buf_p = buf;
}

template <typename U>
void AddOctal(U** buf_p, size_t& maxlen, unsigned int val, int width, int flags)
{
	U text[32];
	int digits = 0;
	U* buf = *buf_p;
	U digit = 0;

	do
	{
		digit = ('0' + val % 8);

		text[digits++] = digit;
		val /= 8;
	} while (val);

	if (!(flags & LADJUST))
	{
		while (digits < width && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			width--;
			maxlen--;
		}
	}

	while (digits-- && maxlen)
	{
		*buf++ = text[digits];
		width--;
		maxlen--;
	}

	if (flags & LADJUST)
	{
		while (width-- && maxlen)
		{
			*buf++ = (flags & ZEROPAD) ? '0' : ' ';
			maxlen--;
		}
	}

	*buf_p = buf;
}

//#define ATCPRINTF_ADVANCE(fmt, ispacked) atcadvance(fmt, ispacked)

template <typename S>
static inline unsigned char atcadvance(unsigned char const** fmt, bool ispacked)
{
	// This code assumes cell alignment.
	char ret = **fmt;
	if (ispacked)
		*fmt = (unsigned char const*)((((uintptr_t)*fmt ^ (sizeof(S) - 1)) + 1) ^ (sizeof(S) - 1));
	else
		(*fmt) += sizeof(S);
	return ret;
}

template <typename D, typename S>
size_t atcprintf(D* buffer, size_t maxlen, const S* format, AMX* amx, const cell* params, int* param)
{
	cell* cptr;
	int arg;
	int args = params[0] / sizeof(cell);
	D* buf_p;
	D ch;
	int flags;
	int width;
	int prec;
	int n;
	// char sign;
	const unsigned char* fmt;
	size_t llen = maxlen;
	bool ispacked = sizeof(S) == sizeof(ucell) && (ucell)*format > UNPACKEDMAX;

	buf_p = buffer;
	arg = *param;
	// Invert the byte order.
	fmt = ispacked ? (unsigned char*)((intptr_t)format + sizeof(S) - 1) : (unsigned char*)format;

	while (true)
	{
		// run through the format string until we hit a '%' or '\0'
		for (ch = static_cast<D>(*fmt);
			 llen && ((ch = static_cast<D>(*fmt)) != '\0' && ch != '%');
			 atcadvance<S>(&fmt, ispacked))
		{
			*buf_p++ = static_cast<D>(ch);
			llen--;
		}
		if (ch == '\0' || llen <= 0)
			goto done;

		// skip over the '%'
		atcadvance<S>(&fmt, ispacked);

		// reset formatting state
		flags = 0;
		width = 0;
		prec = -1;
		// sign = '\0';

rflag:
		ch = static_cast<D>(atcadvance<S>(&fmt, ispacked));
reswitch:
		switch (ch)
		{
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '.':
			ch = static_cast<D>(*fmt);
			if (ch == '*')
			{
				amx_GetAddr(amx, params[arg], &cptr);
				prec = cptr ? *cptr : 0;
				arg++;
				atcadvance<S>(&fmt, ispacked);
				goto rflag;
			}
			else
			{
				n = 0;
				while (is_digit((ch = static_cast<D>(atcadvance<S>(&fmt, ispacked)))))
					n = 10 * n + (ch - '0');
				prec = n < 0 ? -1 : n;
				goto reswitch;
			}
		case '0':
			flags |= ZEROPAD;
			goto rflag;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			n = 0;
			do
			{
				n = 10 * n + (ch - '0');
				ch = static_cast<D>(atcadvance<S>(&fmt, ispacked));
			} while (is_digit(ch));
			width = n;
			goto reswitch;
		case '*':
			amx_GetAddr(amx, params[arg], &cptr);
			width = cptr ? *cptr : 0;
			arg++;
			goto rflag;
		case 'c':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			*buf_p++ = static_cast<D>(cptr ? *cptr : 0);
			llen--;
			arg++;
			break;
		case 'b':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddBinary(&buf_p, llen, cptr ? *cptr : 0, width, flags);
			arg++;
			break;
		case 'o':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddOctal(&buf_p, llen, cptr ? *cptr : 0, width, flags);
			arg++;
			break;
		case 'd':
		case 'i':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddInt(&buf_p, llen, cptr ? *cptr : 0, width, flags);
			arg++;
			break;
		case 'u':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddUInt(&buf_p, llen, static_cast<unsigned int>(cptr ? *cptr : 0), width, flags);
			arg++;
			break;
		case 'f':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddFloat(&buf_p, llen, cptr ? amx_ctof(*cptr) : 0.0f, width, prec, flags);
			arg++;
			break;
		case 'H':
		case 'x':
			CHECK_ARGS(0);
			flags |= UPPERDIGITS;
			amx_GetAddr(amx, params[arg], &cptr);
			AddHex(&buf_p, llen, static_cast<unsigned int>(cptr ? *cptr : 0), width, flags);
			arg++;
			break;
		case 'h':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			AddHex(&buf_p, llen, static_cast<unsigned int>(cptr ? *cptr : 0), width, flags);
			arg++;
			break;
		case 'a':
		{
			CHECK_ARGS(0);
			// %a is passed a pointer directly to a cell string.
			amx_GetAddr(amx, params[arg], &cptr);
			if (!cptr)
			{
				PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid vector string handle provided");
				return 0;
			}
			cell* ptr = reinterpret_cast<cell*>(*cptr);
			if (!ptr)
			{
				PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid vector string handle provided (%d)", *cptr);
				return 0;
			}

			AddString(&buf_p, llen, ptr, width, prec, flags);
			arg++;
			break;
		}
		case 's':
			CHECK_ARGS(0);
			amx_GetAddr(amx, params[arg], &cptr);
			if (cptr)
			{
				AddString(&buf_p, llen, cptr, width, prec, flags);
			}
			arg++;
			break;
		case 'q':
		{
			CHECK_ARGS(0);

			int argLen = 0;
			amx_GetAddr(amx, params[arg], &cptr);
			if (cptr)
			{
				amx_StrLen(cptr, &argLen);
			}
			if (argLen > 0)
			{
				// using only cell
				DynamicArray<cell> escaped(cptr, cptr + argLen);
				cell quote = '\'';

				for (size_t i = 0; i < escaped.size(); ++i)
				{
					if (escaped[i] == quote)
					{
						escaped.insert(escaped.begin() + i, quote);
						i++;
					}
				}

				// null terminator
				escaped.push_back(0);

				AddString(&buf_p, llen, escaped.data(), width, prec, flags);
			}

			arg++;
			break;
		}
		case '%':
			*buf_p++ = static_cast<D>(ch);
			if (!llen)
				goto done;
			llen--;
			break;
		case '\0':
			*buf_p++ = static_cast<D>('%');
			if (!llen)
				goto done;
			llen--;
			goto done;
			break;
		default:
			*buf_p++ = static_cast<D>(ch);
			if (!llen)
				goto done;
			llen--;
			break;
		}
	}

done:
	*buf_p = static_cast<D>(0);
	*param = arg;

	return maxlen - llen;
}

/**
 * HACKHACK: The compiler will generate code for each case we need.
 * Don't remove this, otherwise files that use certain code generations
 *  will have extern problems.  For each case you need, add dummy code
 *  here.
 */
void __WHOA_DONT_CALL_ME_PLZ_K_lol_o_O()
{
	// accprintf
	atcprintf((cell*)NULL, 0, (cell const*)NULL, NULL, NULL, NULL);
	// ascprintf
	atcprintf((char*)NULL, 0, (cell const*)NULL, NULL, NULL, NULL);
}

// StringView printf.
AmxStringFormatter::AmxStringFormatter(cell const* format, AMX* amx, cell const* params, int paramOffset) noexcept
{
	int count = params[0] / sizeof(cell);
	int diff = count - paramOffset;
	int formatLength;
	buf[0] = '\0';
	amx_StrLen(format, &formatLength);
	if (formatLength <= 0 || formatLength >= buf.size())
	{
		// Too big.
		length = 0;
		return;
	}
	if (diff == 0)
	{
		// No formatting.  Fallback.
		amx_GetString(buf.data(), format, false, formatLength + 1);
		length = formatLength;
	}
	else
	{
		// Adjust the offset by 1 to account for the initial hidden `count` parameter.
		++paramOffset;
		length = atcprintf(buf.data(), buf.size() - 1, format, amx, params, &paramOffset);
		if (paramOffset <= count)
		{
			char* fmt;
			if (formatLength > 0 && (fmt = reinterpret_cast<char*>(alloca(formatLength + 1))))
			{
				amx_GetString(fmt, format, false, formatLength + 1);
			}
			else
			{
				fmt = const_cast<char*>("");
			}
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Insufficient specifiers given: \"%s\" does not format %u parameters.", fmt, diff);
		}
	}
}
