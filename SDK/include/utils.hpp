#pragma once

#include "types.hpp"
#include <cctype>

#define STRINGIFY(s) _STRINGIFY(s)
#define _STRINGIFY(s) #s

inline StringView trim(StringView view)
{
	char const* const whitespace = " \t\n\r\f\v";
	const size_t start = view.find_first_not_of(whitespace);
	if (start == StringView::npos)
	{
		return "";
	}
	const size_t end = view.find_last_not_of(whitespace);
	return view.substr(start, end - start + 1);
}

/// Makes Case Insensitive comparison of StringView strings
inline bool strIsEqualCI(const StringView& str1, const StringView& str2)
{
	return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
		[](const char& c1, const char& c2)
		{
			return std::tolower(c1) == std::tolower(c2);
		});
}
