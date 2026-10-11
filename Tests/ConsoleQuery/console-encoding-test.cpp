#include <console_encoding.hpp>
#include <iostream>
#include <stdexcept>

void check(bool value)
{
	if (!value)
		throw std::runtime_error("Console encoding check failed");
}
int main()
{
	std::string result;
	check(ConsoleEncoding::encode(L"say 中文", 936, result) && result == "say \xD6\xD0\xCE\xC4");
	check(ConsoleEncoding::encode(L"say 中英 ABC 123，。！", 936, result) && result == "say \xD6\xD0\xD3\xA2 ABC 123\xA3\xAC\xA1\xA3\xA3\xA1");
	check(ConsoleEncoding::encode(L"say 中文", 65001, result) && result == u8"say 中文");
	check(ConsoleEncoding::encode(L"say 中文", 950, result) && result == "say \xA4\xA4\xA4\xE5");
	check(ConsoleEncoding::toUtf8("\xD6\xD0\xCE\xC4", 936, result) && result == u8"中文");
	check(!ConsoleEncoding::encode(L"say \U0001F600", 936, result) && result.empty());
	check(!ConsoleEncoding::encode(std::wstring(1, wchar_t(0xD800)), 936, result));
	check(!ConsoleEncoding::encode(std::wstring(1, wchar_t(0xD800)), 65001, result));
	check(!ConsoleEncoding::encode(L"say Chinese", 999999, result));
	check(ConsoleEncoding::encode(L"", 936, result) && result.empty());
	check(ConsoleEncoding::encode(std::wstring(8192, L'中'), 936, result) && result.size() == 16384);
	check(ConsoleEncoding::fromUtf8(u8"say 中文 ABC 123", 936, result) && result == "say \xD6\xD0\xCE\xC4 ABC 123");
	check(ConsoleEncoding::fromUtf8(u8"say 中文", 65001, result) && result == u8"say 中文");
	check(!ConsoleEncoding::fromUtf8("say \xD6\xD0\xCE\xC4", 936, result) && result.empty());
	check(!ConsoleEncoding::fromUtf8("\xC0\xAF", 936, result)); // Overlong UTF-8.
	check(!ConsoleEncoding::fromUtf8("\xED\xA0\x80", 936, result)); // Surrogate.
	check(!ConsoleEncoding::fromUtf8("\xF4\x90\x80\x80", 936, result)); // Above U+10FFFF.
	check(!ConsoleEncoding::fromUtf8(u8"say \U0001F600", 936, result) && result.empty());
	check(!ConsoleEncoding::fromUtf8("ASCII", 999999, result));
	check(ConsoleEncoding::fromUtf8("", 936, result) && result.empty());
	std::cout << "PASS: GBK exact bytes, mixed Chinese/ASCII, UTF-8, Big5, reply encoding, unsupported characters, malformed UTF-16, invalid codepage, empty and long input\n";
}
