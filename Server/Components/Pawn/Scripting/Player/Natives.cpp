#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SendClientMessage, bool(IPlayer * player, uint32_t colour, std::string const & msg))
{
	player->sendClientMessage(colour, msg.c_str());
	return true;
}
