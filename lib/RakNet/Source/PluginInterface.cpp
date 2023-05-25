/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "PluginInterface.h"

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning( push )
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnAttach(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnDetach(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnInitialize(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::Update(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
PluginReceiveResult PluginInterface::OnReceive(RakPeerInterface *peer, Packet *packet)
{
	return RR_CONTINUE_PROCESSING;
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnDisconnect(RakPeerInterface *peer)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnCloseConnection(RakPeerInterface *peer, PlayerID playerId)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnDirectSocketSend(const char *data, const unsigned bitsUsed, PlayerID remoteSystemID)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnDirectSocketReceive(const char *data, const unsigned bitsUsed, PlayerID remoteSystemID)
{
}
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
void PluginInterface::OnInternalPacket(InternalPacket *internalPacket, unsigned frameNumber, PlayerID remoteSystemID, RakNetTime time, bool isSend)
{
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
