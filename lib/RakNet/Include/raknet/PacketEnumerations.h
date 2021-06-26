/// \file
/// \brief All the packet identifiers used by RakNet.  Packet identifiers comprise the first byte of any message.
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

#ifndef __PACKET_ENUMERATIONS_H
#define __PACKET_ENUMERATIONS_H 

namespace RakNet
{
	#if RAKNET_LEGACY
	enum
	{
		//
		// https://github.com/oscar-broman/SKY/blob/master/src/RPCs.h
		//
		/// These types are never returned to the user.
		/// 0: ???
		ID_UNKNOWN_0,
		/// 1: ???
		ID_UNKNOWN_1,
		/// 2: ???
		ID_UNKNOWN_2,
		/// 3: ???
		ID_UNKNOWN_3,
		/// 4: ???
		ID_UNKNOWN_4,
		/// 5: ???
		ID_UNKNOWN_5,
		/// 6: Ping from a connected system.  Update timestamps (internal use only)
		ID_INTERNAL_PING,  
		/// 7: Ping from an unconnected system.  Reply but do not update timestamps. (internal use only)
		ID_PING,
		/// 8: Ping from an unconnected system.  Only reply if we have open connections. Do not update timestamps. (internal use only)
		ID_PING_OPEN_CONNECTIONS,
		/// 9: Pong from a connected system.  Update timestamps (internal use only)
		ID_CONNECTED_PONG,
		/// 10: Someone asked for our static data (internal use only)
		ID_REQUEST_STATIC_DATA,
		/// 11: Asking for a new connection (internal use only)
		ID_CONNECTION_REQUEST,
		/// 12: Asking for a new connection (internal use only)
		ID_AUTH_KEY,
		/// 13: ???
		ID_UNKNOWN_13,
		/// 14: Maybe ID_BROADCAST_PINGS
		ID_UNKNOWN_14,
		/// 15: Server / Client only - The server is broadcasting the pings of all players in the game (internal use only)
		ID_BROADCAST_PINGS,
		/// 16: Connecting to a secured server/peer
		ID_SECURED_CONNECTION_RESPONSE,
		/// 17: Connecting to a secured server/peer
		ID_SECURED_CONNECTION_CONFIRMATION,
		/// 18: Packet that tells us the packet contains an integer ID to name mapping for the remote system
		ID_RPC_MAPPING,
		/// 19: Server / Client only - The server is broadcasting a random number seed (internal use only)
		ID_SET_RANDOM_NUMBER_SEED,
		/// 20: Remote procedure call (internal use only)
		ID_RPC,
		/// 21: Remote procedure call reply, for RPCs that return data (internal use only)
		ID_RPC_REPLY,
		/// 22: ???
		ID_UNKNOWN_22,
		/// 23: A reliable packet to detect lost connections
		ID_DETECT_LOST_CONNECTIONS,
		/// 24: Offline message so we know when to reset and start a new connection
		ID_OPEN_CONNECTION_REQUEST,
		/// 25: Offline message response so we know when to reset and start a new connection
		ID_OPEN_CONNECTION_REPLY,
		/// 26:
		ID_OPEN_CONNECTION_COOKIE,
		/// 27: ???
		ID_UNKNOWN_27,
			
		/// [CLIENT|PEER] 28: We preset an RSA public key which does not match what the system we connected to is using.
		ID_RSA_PUBLIC_KEY_MISMATCH,
		//
		// USER TYPES - DO NOT CHANGE THESE
		//
		// Ordered from most useful to least useful
		/// [PEER|SERVER|CLIENT] 29: Sent to the player when a connection request cannot be completed due to inability to connect.
		/// Never transmitted.
		ID_CONNECTION_ATTEMPT_FAILED,

		/// [PEER|SERVER] 30: A remote system has successfully connected.
		ID_NEW_INCOMING_CONNECTION,

		/// [PEER|CLIENT] 31: The system we attempted to connect to is not accepting new connections.
		ID_NO_FREE_INCOMING_CONNECTIONS,

		/// [PEER|SERVER|CLIENT] 32: The system specified in Packet::playerID has disconnected from us.  For the client, this would mean the server has shutdown.
		ID_DISCONNECTION_NOTIFICATION,

		/// [PEER|SERVER|CLIENT] 33: Reliable packets cannot be delivered to the system specifed in Packet::playerID.  The connection to that system has been closed.
		ID_CONNECTION_LOST,

		/// [PEER|CLIENT] 34: In a client/server environment, our connection request to the server has been accepted.
		ID_CONNECTION_REQUEST_ACCEPTED,

		/// [PEER|CLIENT] 35:
		ID_INITIALIZE_ENCRYPTION,

		/// [PEER|CLIENT] 36: We are banned from the system we attempted to connect to.
		ID_CONNECTION_BANNED,

		/// [PEER|CLIENT] 37: The remote system is using a password and has refused our connection because we did not set the correct password.
		ID_INVALID_PASSWORD,

		/// [PEER|SERVER|CLIENT] 38: A packet has been tampered with in transit.  The sender is contained in Packet::playerID.
		/// Never transmitted.
		ID_MODIFIED_PACKET,

		/// [PEER] 39: Pong from an unconnected system.  First byte is ID_PONG, second sizeof(RakNetTime) bytes is the ping, following bytes is system specific enumeration data.
		ID_PONG,

		/// [PEER|SERVER|CLIENT] 40: The four bytes following this byte represent an unsigned int which is automatically modified by the difference in system times between the sender and the recipient. Requires that you call StartOccasionalPing.
		ID_TIMESTAMP,

		/// [PEER|SERVER|CLIENT] 41: We got a bitstream containing static data.  You can now read this data. This packet is transmitted automatically on connections, and can also be manually sent.
		ID_RECEIVED_STATIC_DATA,

		/// [CLIENT] 42: In a client/server environment, a client other than ourselves has disconnected gracefully.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_DISCONNECTION_NOTIFICATION,

		/// [CLIENT] 43: In a client/server environment, a client other than ourselves has been forcefully dropped. Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_CONNECTION_LOST,

		/// [CLIENT] 44: In a client/server environment, a client other than ourselves has connected.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_NEW_INCOMING_CONNECTION,

		/// [CLIENT] 45: On our initial connection to the server, we are told of every other client in the game.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_EXISTING_CONNECTION,

		/// [CLIENT] - 46: Got the data for another client
		ID_REMOTE_STATIC_DATA,

		/// [FILELIST] 47:
		ID_FILE_LIST_TRANSFER_HEADER,

		/// [FILELIST] 48:
		ID_FILE_LIST_TRANSFER_FILE,

		/// [Delta Directory transfer] 49:
		ID_DDT_DOWNLOAD_REQUEST,

		/// [MASTERSERVER] 50: Request to the master server for the list of servers that contain at least one of the specified keys
		ID_QUERY_MASTER_SERVER,
		/// [MASTERSERVER] 51: Remove a game server from the master server.
		ID_MASTER_SERVER_DELIST_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 52: Add or update the information for a server.
		ID_MASTER_SERVER_UPDATE_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 53: Add or set the information for a server.
		ID_MASTER_SERVER_SET_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 54: This message indicates a game client is connecting to a game server, and is relayed through the master server.
		ID_RELAYED_CONNECTION_NOTIFICATION,

		/// [PEER|SERVER|CLIENT] 55: Inform a remote system of our IP/Port.
		ID_ADVERTISE_SYSTEM,

		/// [RakNetTransport] 56
		ID_TRANSPORT_STRING,

		/// [ReplicaManager] 57
		ID_REPLICA_MANAGER_CONSTRUCTION,
		/// [ReplicaManager] 58
		ID_REPLICA_MANAGER_DESTRUCTION,
		/// [ReplicaManager] 59
		ID_REPLICA_MANAGER_SCOPE_CHANGE,
		/// [ReplicaManager] 60
		ID_REPLICA_MANAGER_SERIALIZE,
		/// [ReplicaManager] 61
		ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE,

		/// [ConnectionGraph] 62
		ID_CONNECTION_GRAPH_REQUEST,
		/// [ConnectionGraph] 63
		ID_CONNECTION_GRAPH_REPLY,
		/// [ConnectionGraph] 64
		ID_CONNECTION_GRAPH_UPDATE,
		/// [ConnectionGraph] 65
		ID_CONNECTION_GRAPH_NEW_CONNECTION,
		/// [ConnectionGraph] 66
		ID_CONNECTION_GRAPH_CONNECTION_LOST,
		/// [ConnectionGraph] 67
		ID_CONNECTION_GRAPH_DISCONNECTION_NOTIFICATION,

		/// [Router] 68
		ID_ROUTE_AND_MULTICAST,

		/// [RakVoice] 69
		ID_RAKVOICE_OPEN_CHANNEL_REQUEST,
		/// [RakVoice] 70
		ID_RAKVOICE_OPEN_CHANNEL_REPLY,
		/// [RakVoice] 71
		ID_RAKVOICE_CLOSE_CHANNEL,
		/// [RakVoice] 72
		ID_RAKVOICE_DATA,

		/// [Autopatcher] 73
		ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE,
		ID_AUTOPATCHER_CREATION_LIST,
		ID_AUTOPATCHER_DELETION_LIST,
		ID_AUTOPATCHER_GET_PATCH,
		ID_AUTOPATCHER_PATCH_LIST,
		ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR, // Returned to user
		ID_AUTOPATCHER_FINISHED,
		ID_AUTOPATCHER_RESTART_APPLICATION, // Returned to user

		/// [NAT Punchthrough]
		ID_NAT_PUNCHTHROUGH_REQUEST,

		/// [NAT Punchthrough] Returned to user.  PlayerID binary address / port is written to the stream
		ID_NAT_TARGET_NOT_CONNECTED,

		/// [NAT Punchthrough] Returned to user.  PlayerID binary address / port is written to the stream
		ID_NAT_TARGET_CONNECTION_LOST,

		// [NAT Punchthrough]
		ID_NAT_CONNECT_AT_TIME,

		// [NAT Punchthrough]
		ID_NAT_SEND_OFFLINE_MESSAGE_AT_TIME,

		// [Database] Internal
		ID_DATABASE_QUERY_REQUEST,

		// [Database] Internal
		ID_DATABASE_UPDATE_ROW,

		// [Database] Internal
		ID_DATABASE_REMOVE_ROW,

		// [Database] A serialized table.  Bytes 1+ contain the table.  Pass to TableSerializer::DeserializeTable
		ID_DATABASE_QUERY_REPLY,

		// [Database] Specified table not found
		ID_DATABASE_UNKNOWN_TABLE,

		// [Database] Incorrect password
		ID_DATABASE_INCORRECT_PASSWORD,

		// RakPeer - Downloading a large message. Format is ID_DOWNLOAD_PROGRESS (MessageID), partCount (unsigned int), partTotal (unsigned int), partLength (unsigned int), first part data (length <= MAX_MTU_SIZE)
		ID_DOWNLOAD_PROGRESS,
		
		/// Depreciated
		ID_RESERVED9,
		// For the user to use.  Start your first enumeration at this value.
		ID_USER_PACKET_ENUM,
		//-------------------------------------------------------------------------------------------------------------
	 
	};
	#else
	/// You should not edit the file PacketEnumerations.h as it is a part of RakNet static library
	/// To define your own message id, define an enum following the code example that follows. 
	///
	/// \code
	/// enum {
	///   ID_MYPROJECT_MSG_1 = ID_USER_PACKET_ENUM
	///   ID_MYPROJECT_MSG_2, 
	///    ... 
	/// };
	/// \endcode 
	///
	/// \note All these enumerations should be casted to (unsigned char) before writing them to RakNet::BitStream
	enum
	{
		//
		// RESERVED TYPES - DO NOT CHANGE THESE
		//
		/// These types are never returned to the user.
		/// 0: Ping from a connected system.  Update timestamps (internal use only)
		ID_INTERNAL_PING,  
		/// 1: Ping from an unconnected system.  Reply but do not update timestamps. (internal use only)
		ID_PING,
		/// 2: Ping from an unconnected system.  Only reply if we have open connections. Do not update timestamps. (internal use only)
		ID_PING_OPEN_CONNECTIONS,
		/// 3: Pong from a connected system.  Update timestamps (internal use only)
		ID_CONNECTED_PONG,
		/// 4: Someone asked for our static data (internal use only)
		ID_REQUEST_STATIC_DATA,
		/// 5: Asking for a new connection (internal use only)
		ID_CONNECTION_REQUEST,
		/// 6: Connecting to a secured server/peer
		ID_SECURED_CONNECTION_RESPONSE,
		/// 7: Connecting to a secured server/peer
		ID_SECURED_CONNECTION_CONFIRMATION,
		/// 8: Packet that tells us the packet contains an integer ID to name mapping for the remote system
		ID_RPC_MAPPING,
		/// 9: A reliable packet to detect lost connections
		ID_DETECT_LOST_CONNECTIONS,
		/// 10: Offline message so we know when to reset and start a new connection
		ID_OPEN_CONNECTION_REQUEST,
		/// 11: Offline message response so we know when to reset and start a new connection
		ID_OPEN_CONNECTION_REPLY,
		/// 12: Remote procedure call (internal use only)
		ID_RPC,
		/// 13: Remote procedure call reply, for RPCs that return data (internal use only)
		ID_RPC_REPLY,
		/// 14: Server / Client only - The server is broadcasting the pings of all players in the game (internal use only)
		ID_BROADCAST_PINGS,
		/// 15: Server / Client only - The server is broadcasting a random number seed (internal use only)
		ID_SET_RANDOM_NUMBER_SEED,

		//
		// USER TYPES - DO NOT CHANGE THESE
		//
		// Ordered from most useful to least useful

		/// [PEER|CLIENT] 16: In a client/server environment, our connection request to the server has been accepted.
		ID_CONNECTION_REQUEST_ACCEPTED,

		/// [PEER|SERVER|CLIENT] 17: Sent to the player when a connection request cannot be completed due to inability to connect.
		/// Never transmitted.
		ID_CONNECTION_ATTEMPT_FAILED,

		/// [PEER|SERVER] 18: A remote system has successfully connected.
		ID_NEW_INCOMING_CONNECTION,

		/// [PEER|CLIENT] 19: The system we attempted to connect to is not accepting new connections.
		ID_NO_FREE_INCOMING_CONNECTIONS,

		/// [PEER|SERVER|CLIENT] 20: The system specified in Packet::playerID has disconnected from us.  For the client, this would mean the server has shutdown.
		ID_DISCONNECTION_NOTIFICATION,

		/// [PEER|SERVER|CLIENT] 21: Reliable packets cannot be delivered to the system specifed in Packet::playerID.  The connection to that system has been closed.
		ID_CONNECTION_LOST,

		/// [CLIENT|PEER] 22: We preset an RSA public key which does not match what the system we connected to is using.
		ID_RSA_PUBLIC_KEY_MISMATCH,

		/// [PEER|CLIENT] 23: We are banned from the system we attempted to connect to.
		ID_CONNECTION_BANNED,

		/// [PEER|CLIENT] 24: The remote system is using a password and has refused our connection because we did not set the correct password.
		ID_INVALID_PASSWORD,

		/// [PEER|SERVER|CLIENT] 25: A packet has been tampered with in transit.  The sender is contained in Packet::playerID.
		/// Never transmitted.
		ID_MODIFIED_PACKET,

		/// [PEER|SERVER|CLIENT] 26: The four bytes following this byte represent an unsigned int which is automatically modified by the difference in system times between the sender and the recipient. Requires that you call StartOccasionalPing.
		ID_TIMESTAMP,

		/// [PEER] 27: Pong from an unconnected system.  First byte is ID_PONG, second sizeof(RakNetTime) bytes is the ping, following bytes is system specific enumeration data.
		ID_PONG,

		/// [PEER|SERVER|CLIENT] 28: We got a bitstream containing static data.  You can now read this data. This packet is transmitted automatically on connections, and can also be manually sent.
		ID_RECEIVED_STATIC_DATA,

		/// [CLIENT] 29: In a client/server environment, a client other than ourselves has disconnected gracefully.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_DISCONNECTION_NOTIFICATION,

		/// [CLIENT] 30: In a client/server environment, a client other than ourselves has been forcefully dropped. Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_CONNECTION_LOST,

		/// [CLIENT] 31: In a client/server environment, a client other than ourselves has connected.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_NEW_INCOMING_CONNECTION,

		/// [CLIENT] 32: On our initial connection to the server, we are told of every other client in the game.  Packet::playerID is modified to reflect the playerID of this client.
		ID_REMOTE_EXISTING_CONNECTION,

		/// [CLIENT] - 33: Got the data for another client
		ID_REMOTE_STATIC_DATA,

		/// [FILELIST] 34:
		ID_FILE_LIST_TRANSFER_HEADER,

		/// [FILELIST] 35:
		ID_FILE_LIST_TRANSFER_FILE,

		/// [Delta Directory transfer] 36:
		ID_DDT_DOWNLOAD_REQUEST,

		/// [MASTERSERVER] 37: Request to the master server for the list of servers that contain at least one of the specified keys
		ID_QUERY_MASTER_SERVER,
		/// [MASTERSERVER] 38: Remove a game server from the master server.
		ID_MASTER_SERVER_DELIST_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 39: Add or update the information for a server.
		ID_MASTER_SERVER_UPDATE_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 40: Add or set the information for a server.
		ID_MASTER_SERVER_SET_SERVER,
		/// [MASTERSERVER|MASTERCLIENT] 41: This message indicates a game client is connecting to a game server, and is relayed through the master server.
		ID_RELAYED_CONNECTION_NOTIFICATION,

		/// [PEER|SERVER|CLIENT] 42: Inform a remote system of our IP/Port.
		ID_ADVERTISE_SYSTEM,

		/// [RakNetTransport] 43
		ID_TRANSPORT_STRING,

		/// [ReplicaManager] 44
		ID_REPLICA_MANAGER_CONSTRUCTION,
		/// [ReplicaManager] 45
		ID_REPLICA_MANAGER_DESTRUCTION,
		/// [ReplicaManager] 46
		ID_REPLICA_MANAGER_SCOPE_CHANGE,
		/// [ReplicaManager] 47
		ID_REPLICA_MANAGER_SERIALIZE,
		/// [ReplicaManager] 48
		ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE,

		/// [ConnectionGraph] 49
		ID_CONNECTION_GRAPH_REQUEST,
		/// [ConnectionGraph] 50
		ID_CONNECTION_GRAPH_REPLY,
		/// [ConnectionGraph] 51
		ID_CONNECTION_GRAPH_UPDATE,
		/// [ConnectionGraph] 52
		ID_CONNECTION_GRAPH_NEW_CONNECTION,
		/// [ConnectionGraph] 53
		ID_CONNECTION_GRAPH_CONNECTION_LOST,
		/// [ConnectionGraph] 54
		ID_CONNECTION_GRAPH_DISCONNECTION_NOTIFICATION,

		/// [Router] 55
		ID_ROUTE_AND_MULTICAST,

		/// [RakVoice] 56
		ID_RAKVOICE_OPEN_CHANNEL_REQUEST,
		/// [RakVoice] 57
		ID_RAKVOICE_OPEN_CHANNEL_REPLY,
		/// [RakVoice] 58
		ID_RAKVOICE_CLOSE_CHANNEL,
		/// [RakVoice] 59
		ID_RAKVOICE_DATA,

		/// [Autopatcher] 60
		ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE,
		ID_AUTOPATCHER_CREATION_LIST,
		ID_AUTOPATCHER_DELETION_LIST,
		ID_AUTOPATCHER_GET_PATCH,
		ID_AUTOPATCHER_PATCH_LIST,
		ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR, // Returned to user
		ID_AUTOPATCHER_FINISHED,
		ID_AUTOPATCHER_RESTART_APPLICATION, // Returned to user

		/// [NAT Punchthrough]
		ID_NAT_PUNCHTHROUGH_REQUEST,

		/// [NAT Punchthrough] Returned to user.  PlayerID binary address / port is written to the stream
		ID_NAT_TARGET_NOT_CONNECTED,

		/// [NAT Punchthrough] Returned to user.  PlayerID binary address / port is written to the stream
		ID_NAT_TARGET_CONNECTION_LOST,

		// [NAT Punchthrough]
		ID_NAT_CONNECT_AT_TIME,

		// [NAT Punchthrough]
		ID_NAT_SEND_OFFLINE_MESSAGE_AT_TIME,

		// [Database] Internal
		ID_DATABASE_QUERY_REQUEST,

		// [Database] Internal
		ID_DATABASE_UPDATE_ROW,

		// [Database] Internal
		ID_DATABASE_REMOVE_ROW,

		// [Database] A serialized table.  Bytes 1+ contain the table.  Pass to TableSerializer::DeserializeTable
		ID_DATABASE_QUERY_REPLY,

		// [Database] Specified table not found
		ID_DATABASE_UNKNOWN_TABLE,

		// [Database] Incorrect password
		ID_DATABASE_INCORRECT_PASSWORD,

		// RakPeer - Downloading a large message. Format is ID_DOWNLOAD_PROGRESS (MessageID), partCount (unsigned int), partTotal (unsigned int), partLength (unsigned int), first part data (length <= MAX_MTU_SIZE)
		ID_DOWNLOAD_PROGRESS,
		
		/// Depreciated
		ID_RESERVED9,
		// For the user to use.  Start your first enumeration at this value.
		ID_USER_PACKET_ENUM,
		//-------------------------------------------------------------------------------------------------------------
	 
	};
	#endif
}

#endif
