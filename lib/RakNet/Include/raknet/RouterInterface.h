#ifndef __ROUTER_INTERFACE_H
#define __ROUTER_INTERFACE_H

#include "Export.h"

namespace RakNet
{
	/// On failed directed sends, RakNet can call an alternative send function to use.
	class RAK_DLL_EXPORT RouterInterface
	{
	public:
		virtual bool Send( const char *data, unsigned bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId )=0;
	};
}

#endif
