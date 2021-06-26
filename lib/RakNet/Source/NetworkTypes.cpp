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

#include "NetworkTypes.h"
#include <string.h>
#include <stdio.h>

#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h"
#elif defined(_WIN32)
// IP_DONTFRAGMENT is different between winsock 1 and winsock 2.  Therefore, Winsock2.h must be linked againt Ws2_32.lib
// winsock.h must be linked against WSock32.lib.  If these two are mixed up the flag won't work correctly
#include <winsock2.h>
#include <stdlib.h> // itoa
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

using namespace RakNet;

// Fast itoa from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html for Linux since it seems like Linux doesn't support this function.
// I modified it to remove the std dependencies.
char* my_itoa( int value, char* result, int base ) {
	// check that the base if valid
	if (base < 2 || base > 16) { *result = 0; return result; }
	char* out = result;
	int quotient = value;

	int absQModB;

	do {
		// KevinJ - get rid of this dependency
		//*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
		absQModB=quotient % base;
		if (absQModB < 0)
			absQModB=-absQModB;
		*out = "0123456789abcdef"[ absQModB ];
		++out;
		quotient /= base;
	} while ( quotient );

	// Only apply negative sign for base 10
	if ( value < 0 && base == 10) *out++ = '-';

	// KevinJ - get rid of this dependency
	// std::reverse( result, out );
	*out = 0;

	// KevinJ - My own reverse code
    char *start = result;
	char temp;
	out--;
	while (start < out)
	{
		temp=*start;
		*start=*out;
		*out=temp;
		start++;
		out--;
	}

	return result;
}

// Defaults to not in peer to peer mode for NetworkIDs.  This only sends the localSystemId portion in the BitStream class
// This is what you want for client/server, where the server assigns all NetworkIDs and it is unnecessary to transmit the full structure.
// For peer to peer, this will transmit the playerId of the system that created the object in addition to localSystemId.  This allows
// Any system to create unique ids locally.
// All systems must use the same value for this variable.
bool RAK_DLL_EXPORT NetworkID::peerToPeerMode=false;

bool PlayerID::operator==( const PlayerID& right ) const
{
	return binaryAddress == right.binaryAddress && port == right.port;
}

bool PlayerID::operator!=( const PlayerID& right ) const
{
	return binaryAddress != right.binaryAddress || port != right.port;
}

bool PlayerID::operator>( const PlayerID& right ) const
{
	return ( ( binaryAddress > right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port > right.port ) ) );
}

bool PlayerID::operator<( const PlayerID& right ) const
{
	return ( ( binaryAddress < right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port < right.port ) ) );
}
char *PlayerID::ToString(bool writePort) const
{
#ifdef _COMPATIBILITY_1
	return "";
#else
	static char str[22];
	in_addr in;
	in.s_addr = binaryAddress;
	strcpy(str, inet_ntoa( in ));
	if (writePort)
	{
		strcat(str, ":");
#if (defined(__GNUC__)  || defined(__GCCXML__))
		my_itoa(port, str+strlen(str), 10);
#else
		_itoa(port, str+strlen(str), 10);
#endif
	}
	
	return (char*) str;
#endif
}
void PlayerID::SetBinaryAddress(const char *str)
{
#ifdef _COMPATIBILITY_1
	binaryAddress=UNASSIGNED_PLAYER_ID.binaryAddress;
#else
	binaryAddress=inet_addr(str);
#endif
}

NetworkID& NetworkID::operator = ( const NetworkID& input )
{
	playerId = input.playerId;
	localSystemId = input.localSystemId;
	return *this;
}

bool NetworkID::operator==( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return playerId == right.playerId && localSystemId == right.localSystemId;
	else
		return localSystemId==right.localSystemId;
}

bool NetworkID::operator!=( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return playerId != right.playerId || localSystemId != right.localSystemId;
	else
		return localSystemId!=right.localSystemId;
}

bool NetworkID::operator>( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return ( ( playerId > right.playerId ) || ( ( playerId == right.playerId ) && ( localSystemId > right.localSystemId ) ) );
	else
		return localSystemId>right.localSystemId;
}

bool NetworkID::operator<( const NetworkID& right ) const
{
	if (NetworkID::peerToPeerMode)
		return ( ( playerId < right.playerId ) || ( ( playerId == right.playerId ) && ( localSystemId < right.localSystemId ) ) );
	else
		return localSystemId<right.localSystemId;
}

bool NetworkID::IsPeerToPeerMode(void)
{
	return peerToPeerMode;
}
void NetworkID::SetPeerToPeerMode(bool isPeerToPeer)
{
	peerToPeerMode=isPeerToPeer;
}
