#include <sdk.hpp>
#include "../Types.hpp"

SCRIPT_API(HTTP, bool(int index, int method, std::string const& url, std::string const& data, std::string const& callback))
{
	throw pawn_natives::NotImplemented();
}
