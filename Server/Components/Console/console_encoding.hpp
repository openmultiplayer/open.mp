/* SPDX-License-Identifier: MPL-2.0
 * Lossless Windows console command conversion.
 */
#pragma once
#ifdef WIN32
#include <windows.h>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace ConsoleEncoding
{
// Refuse lossy conversion: a command must not silently acquire '?' or best-fit characters.
inline bool encode(std::wstring_view input, unsigned codepage, std::string& output)
{
	output.clear();
	if (codepage == 0)
		codepage = GetACP();
	if (!IsValidCodePage(codepage) || input.size() > (std::numeric_limits<int>::max)())
		return false;
	if (input.empty())
		return true;
	const DWORD flags = (codepage == CP_UTF8 || codepage == 54936) ? WC_ERR_INVALID_CHARS : 0;
	const int length = static_cast<int>(input.size());
	const int size = WideCharToMultiByte(codepage, flags, input.data(), length, nullptr, 0, nullptr, nullptr);
	if (size <= 0)
		return false;
	std::string result(size, '\0');
	if (WideCharToMultiByte(codepage, flags, input.data(), length, result.data(), size, nullptr, nullptr) != size)
		return false;
	const int wideSize = MultiByteToWideChar(codepage, 0, result.data(), size, nullptr, 0);
	if (wideSize != length)
		return false;
	std::wstring roundTrip(wideSize, L'\0');
	if (MultiByteToWideChar(codepage, 0, result.data(), size, roundTrip.data(), wideSize) != wideSize || roundTrip != input)
		return false;
	output = std::move(result);
	return true;
}

inline bool toUtf8(std::string_view input, unsigned codepage, std::string& output)
{
	output.clear();
	if (input.size() > (std::numeric_limits<int>::max)())
		return false;
	if (input.empty())
		return true;
	const int size = MultiByteToWideChar(codepage, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);
	if (size <= 0)
		return false;
	std::wstring wide(size, L'\0');
	if (MultiByteToWideChar(codepage, 0, input.data(), static_cast<int>(input.size()), wide.data(), size) != size)
		return false;
	return encode(wide, CP_UTF8, output);
}

// Remote RCON has no encoding marker. Only call this when an administrator
// explicitly configures UTF-8 input; never guess from the command bytes.
inline bool fromUtf8(std::string_view input, unsigned codepage, std::string& output)
{
	output.clear();
	if (input.size() > (std::numeric_limits<int>::max)())
		return false;
	if (input.empty())
		return encode({}, codepage, output);
	const int length = static_cast<int>(input.size());
	const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), length, nullptr, 0);
	if (size <= 0)
		return false;
	std::wstring wide(size, L'\0');
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), length, wide.data(), size) != size)
		return false;
	return encode(wide, codepage, output);
}
}
#endif
