/// \file
/// \brief Factory class for RakServerInterface, RakClientInterface, and RakPeerInterface
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

#ifndef __RAK_NETWORK_FACTORY_H
#define __RAK_NETWORK_FACTORY_H

#include "Export.h"

namespace RakNet
{
	class RakClientInterface;
	class RakServerInterface;
	class RakPeerInterface;
	class ConsoleServer;
	//class ReplicaManager;
	//class LogCommandParser;
	//class PacketLogger;
	class RakNetCommandParser;
	//class RakNetTransport;
	//class TelnetTransport;
	//class PacketConsoleLogger;
	//class PacketFileLogger;
	//class Router;
	//class ConnectionGraph;

	class RAK_DLL_EXPORT RakNetworkFactory
	{
	public:
		// For DLL's, these are user classes that you might want to new and delete.
		// You can't instantiate exported classes directly in your program.  The instantiation
		// has to take place inside the DLL.  So these functions will do the news and deletes for you.
		// if you're using the source or static library you don't need these functions, but can use them if you want.
		static RakClientInterface* GetRakClientInterface( void );
		static RakServerInterface* GetRakServerInterface( void );
		static RakPeerInterface* GetRakPeerInterface( void );
		static ConsoleServer* GetConsoleServer( void );
		//static ReplicaManager* GetReplicaManager( void );
		//static LogCommandParser* GetLogCommandParser( void );
		//static PacketLogger* GetPacketLogger( void );
		static RakNetCommandParser* GetRakNetCommandParser( void );
		//static RakNetTransport* GetRakNetTransport( void );
		//static TelnetTransport* GetTelnetTransport( void );
		//static PacketConsoleLogger* GetPacketConsoleLogger( void );
		//static PacketFileLogger* GetPacketFileLogger( void );
		//static Router* GetRouter( void );
		//static ConnectionGraph* GetConnectionGraph( void );

		// To delete the object returned by the Get functions above.
		static void DestroyRakClientInterface( RakClientInterface* i );
		static void DestroyRakServerInterface( RakServerInterface* i );
		static void DestroyRakPeerInterface( RakPeerInterface* i );
		static void DestroyConsoleServer( ConsoleServer* i);
		//static void DestroyReplicaManager( ReplicaManager* i);
		//static void DestroyLogCommandParser( LogCommandParser* i);
		//static void DestroyPacketLogger( PacketLogger* i);
		static void DestroyRakNetCommandParser(  RakNetCommandParser* i );
		//static void DestroyRakNetTransport(  RakNetTransport* i );
		//static void DestroyTelnetTransport(  TelnetTransport* i );
		//static void DestroyPacketConsoleLogger(  PacketConsoleLogger* i );
		//static void DestroyPacketFileLogger(  PacketFileLogger* i );
		//static void DestroyRouter(  Router* i );
		//static void DestroyConnectionGraph(  ConnectionGraph* i );
	};
}

#endif
