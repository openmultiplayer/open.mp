/// \file
/// \brief Contains RakNetTransportCommandParser and RakNetTransport used to provide a secure console connection.
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

#ifndef __RAKNET_TRANSPORT
#define __RAKNET_TRANSPORT

#include "TransportInterface.h"
#include "DS_Queue.h"
#include "CommandParserInterface.h"
#include "Export.h"

namespace RakNet
{
	class RakPeerInterface;
	class RakNetTransport;
	class BitStream;

	/// \brief RakNetTransport has its own command parser to enable remote users to change the command console's password.
	class RAK_DLL_EXPORT RakNetTransportCommandParser : public CommandParserInterface
	{
	public:
		RakNetTransportCommandParser();
		~RakNetTransportCommandParser();

		/// Given \a command with parameters \a parameterList , do whatever processing you wish.
		/// \param[in] command The command to process
		/// \param[in] numParameters How many parameters were passed along with the command
		/// \param[in] parameterList The list of parameters.  parameterList[0] is the first parameter and so on.
		/// \param[in] transport The transport interface we can use to write to
		/// \param[in] playerId The player that sent this command.
		/// \param[in] originalString The string that was actually sent over the network, in case you want to do your own parsing
		bool OnCommand(const char *command, unsigned numParameters, char **parameterList, TransportInterface *transport, PlayerID playerId, const char *originalString);

		/// You are responsible for overriding this function and returning a static string, which will identifier your parser.
		/// This should return a static string
		/// \return The name that you return.
		char* GetName(void) const;

		/// A callback for when you are expected to send a brief description of your parser to \a playerId
		/// \param[in] transport The transport interface we can use to write to
		/// \param[in] playerId The player that requested help.
		void SendHelp(TransportInterface *transport, PlayerID playerId);
	protected:
	};

	/// \brief Use RakNetTransport if you need a secure connection between the client and the console server.
	/// RakNetTransport automatically initializes security for the system.  Use the project CommandConsoleClient to connect
	/// To the ConsoleServer if you use RakNetTransport
	class RAK_DLL_EXPORT RakNetTransport : public TransportInterface
	{
	public:
		RakNetTransport();
		~RakNetTransport();

		/// Start the transport provider on the indicated port.
		/// \param[in] port The port to start the transport provider on
		/// \param[in] serverMode If true, you should allow incoming connections (I don't actually use this anywhere)
		/// \return Return true on success, false on failure.
		bool Start(unsigned short port, bool serverMode);

		/// Stop the transport provider.  You can clear memory and shutdown threads here.
		void Stop(void);

		/// Send a null-terminated string to \a playerId
		/// If your transport method requires particular formatting of the outgoing data (e.g. you don't just send strings) you can do it here
		/// and parse it out in Receive().
		/// \param[in] playerId The player to send the string to
		/// \param[in] data format specifier - same as printf
		/// \param[in] ... format specification arguments - same as printf
		void Send( PlayerID playerId, const char *data, ... );

		/// Return a string. The string should be allocated and written to Packet::data .
		/// The byte length should be written to Packet::length .  The player/address should be written to Packet::playerid
		/// If your transport protocol adds special formatting to the data stream you should parse it out before returning it in the packet
		/// and thus only return a string in Packet::data
		/// \return The packet structure containing the result of Receive, or 0 if no data is available
		Packet* Receive( void );

		/// Deallocate the Packet structure returned by Receive
		/// \param[in] The packet to deallocate
		void DeallocatePacket( Packet *packet );

		/// Disconnect \a playerId .  The binary address and port defines the PlayerID structure.
		/// \param[in] playerId The player/address to disconnect
		void CloseConnection( PlayerID playerId );

		/// If a new system connects to you, you should queue that event and return the playerId/address of that player in this function.
		/// \return The PlayerID/address of the system
		PlayerID HasNewConnection(void);

		/// If a system loses the connection, you should queue that event and return the playerId/address of that player in this function.
		/// \return The PlayerID/address of the system
		PlayerID HasLostConnection(void);

		/// Sets the password which incoming connections must match.
		/// While not required, it is highly recommended you set this in a real game environment or anyone can login and control your server.
		/// Don't set it to a fixed value, but instead require that the server admin sets it when you start the application server
		/// \param[in] password Null-terminated string to use as a password.
		void SetIncomingPassword(const char *password);

		/// Returns the password set by SetIncomingPassword().
		/// \return The password set by SetIncomingPassword()
		char * GetIncomingPassword(void);

		/// Returns RakNetTransportCommandParser so the console admin can change the password
		CommandParserInterface* GetCommandParser(void);

	protected:
		RakPeerInterface *rakPeer;
		void AutoAllocate(void);
		DataStructures::Queue<PlayerID> newConnections, lostConnections;
		RakNetTransportCommandParser rakNetTransportCommandParser;
	};
}

#endif
