#pragma once

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
