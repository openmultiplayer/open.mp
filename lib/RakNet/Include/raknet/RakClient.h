/// \file
/// \brief Specializes RakPeer to act as a client.
///
/// \remarks This file originated from my first game, where I only had a server and a client.  Now that I have RakPeer, it's depreciated although in such wide use I can't delete it.
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

#ifndef __RAK_CLIENT_H
#define __RAK_CLIENT_H

#include "RakPeer.h"
#include "RakClientInterface.h" 

// #pragma deprecated(RakClient)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

namespace RakNet
{
	/// This is a user-interface class to act as a game client.  All it does is implement some functionality on top of RakPeer.
	/// See the individual functions for what the class can do.
	/// \brief Defines the functions used by a game client
	class RakClient : public RakPeer, public RakClientInterface
	{

	public:
		///Constructor
		RakClient();
		
		/// Destructor
		virtual ~RakClient();

		///Call this to connect the client to the specified host (ip or domain name) and server port.
		/// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
		/// or receive gets a packet with the type identifier ID_CONNECTION_REQUEST_ACCEPTED.
		/// \param[in] host Dotted IP or a domain name
		/// \param[in] serverPort The port on which to connect to \a host 
		/// \param[in] clientPort The port to use localy 
		/// \param[in] depreciated Ignore this
		/// \param[in] threadSleepTimer How many ms to Sleep each internal update cycle (30 to give the game priority, 0 for regular (recommended), -1 to not Sleep() (may be slower))
		/// \return true on successful initiation, false otherwise
		bool Connect( const char* host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer ) override;

		/// Stops the client, stops synchronized data, and resets all internal data.
		/// Does nothing if the client is not connected to begin wit
		/// \param[in] blockDuration how long you should wait for all remaining packets to go outIf you set it to 0 then the disconnection notification probably won't arrive
		/// \param[in] orderingChannel If blockDuration > 0, the disconnect packet will be sent on this channel
		void Disconnect( unsigned int blockDuration, unsigned char orderingChannel=0 ) override;
		
		/// Can be called to use specific public RSA keys. (e and n)
		/// In order to prevent altered keys.
		/// Will return ID_RSA_PUBLIC_KEY_MISMATCH in a packet if a key has been altered.
		/// \param[in] privKeyP Private keys generated from the RSACrypt class. Can be 0
		/// \param[in] privKeyQ Private keys generated from the RSACrypt class. Can be 0 
		/// \sa Encryption.cpp	
		void InitializeSecurity( const char *privKeyP, const char *privKeyQ ) override;
		
		/// Set the password to use when connecting to a server.  The password persists between connections.
		/// \param[in] _password The password to use to connect to a server, or 0 for none.
		void SetPassword( const char *_password ) override;
		
		/// Returns true if a password was set, false otherwise
		/// \return true if a password has previously been set using SetPassword
		bool HasPassword( void ) const override;
		
		/// Sends a block of data to the specified system that you are connected to.
		/// This function only works while the connected (Use the Connect function).
		/// \param[in] data The block of data to send
		/// \param[in] length The size in bytes of the data to send
		/// \param[in] priority What priority level to send on.
		/// \param[in] reliability How reliability to send this data
		/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.- Packets are only ordered relative to other packets on the same stream
		/// \return False if we are not connected to the specified recipient.  True otherwise
		bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel ) override;
		
		/// Sends a block of data to the specified system that you are connected to.
		/// This function only works while the connected (Use the Connect function).
		/// \param[in] bitStream The bitstream to send
		/// \param[in] priority What priority level to send on.
		/// \param[in] reliability How reliability to send this data
		/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.- Packets are only ordered relative to other packets on the same stream
		/// \return False if we are not connected to the specified recipient.  True otherwise
		bool Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel ) override;
		
		/// Gets a packet from the incoming packet queue.
		/// Use DeallocatePacket() to deallocate the packet after you are done with it.
		/// User-thread functions, such as RPC calls and the plugin function PluginInterface::Update occur here.
		/// \return 0 if no packets are waiting to be handled, otherwise a pointer to a packet.
		// sa CoreNetworkStructures.h contains struct Packet
		Packet* Receive( void ) override;
		
		/// Call this to deallocate a packet returned by Receive when you are done handling it.
		/// \param[in] packet The packet to deallocate.	
		void DeallocatePacket( Packet *packet ) override;
		
		
		/// Send a ping to the server
		void PingServer( void ) override;
		
		/// Sends a ping request to a server we are not connected to.  This will also initialize the
		/// networking system if it is not already initialized.  You can stop the networking system
		/// by calling Disconnect()
		/// The final ping time will be encoded in the following sizeof(RakNetTime) bytes.  (Default is 4 bytes - See __GET_TIME_64BIT in NetworkTypes.h
		/// You can specify if the server should only reply if it has an open connection or not
		/// This must be true for LAN broadcast server discovery on "255.255.255.255"
		/// or you will get replies from clients as well.
		/// \param[in] host The host to contact 
		/// \param[in] ServerPort the port used by the server
		/// \param[in] clientPort the port used to receive the answer 
		/// \param[in] onlyReplyOnAcceptingConnections if true the server must be ready to accept incomming connection. 
		void PingServer( const char* host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections ) override;
		
		/// Returns the average of all ping times read.
		int GetAveragePing( void ) override;
		
		/// Returns the last ping time read
		/// \return last ping value
		int GetLastPing( void ) const override;
		
		/// Returns the lowest ping time read or -1 if none read yet
		/// \return lowest ping value 
		int GetLowestPing( void ) const override;
		
		/// Returns the last ping for the specified player. This information
		/// is broadcast by the server automatically In order to save
		/// bandwidth this information is updated only infrequently and only
		/// for the first 32 players 
		/// \param[in] playerId The id of the player you want to have the ping (it might be your id) 
		/// \return the last ping for this player 
		/// \note You can read your own ping with
		/// this method by passing your own playerId, however for more
		/// up-to-date readings you should use one of the three functions
		/// above
		int GetPlayerPing( const PlayerID playerId ) override;
		
		/// Ping the server every so often.  This is on by default.
		/// In games where you don't care about ping you can callStopOccasionalPing to save the bandwidth.
		/// This can be called anytime.
		void StartOccasionalPing( void ) override;
		
		/// Stop pinging the server every so often. The server is pinged by
		///  default.  In games where you don't care about ping you can call
		///  this to save the bandwidth This will work anytime
		void StopOccasionalPing( void ) override;
		
		/// Returns true if the client is connected to a responsive server
		/// \return true if connected to a server 
		bool IsConnected( void ) const override;
		
		/// Returns a number automatically synchronized between the server and client which randomly changes every 9 seconds.
		/// The time it changes is accurate to within a few ms and is best used to seed random number generators that you want to usually
		/// return the same output on all systems.
		/// Keep in mind thisisn't perfectly accurate as there is always a very small chance the numbers will by out of synch.
		/// You should should confine its use to visual effects or functionality that has a backup method to maintain synchronization.
		/// If you don't need this functionality and want to save the bandwidth callStopSynchronizedRandomInteger after starting the server
		/// \return A number, which is probably synchronized among all clients and the server.
		unsigned int GetSynchronizedRandomInteger( void ) const override;
		
		/// This is an optional function to generate the compression layer based on the input frequency table.
		/// If you want to use it you should call this twice - once with inputLayer as true and once as false.
		/// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
		/// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true.
		/// Calling this function when there is an existing layer will overwrite the old layer.
		/// \pre You should only call this when disconnected
		/// \param[in] inputFrequencyTable A frequency table for your data
		/// \param[in] inputLayer Is this the input layer?
		/// \return false (failure) if connected.  Otherwise true (success)
		/// \sa Compression.cpp
		bool GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer ) override;
		
		/// Delete the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory.
		/// \pre You should only call this when disconnected
		/// \param[in] inputLayer True to mean the inputLayer, false to mean the output layer
		/// \return false (failure) if connected.  Otherwise true (success)
		bool DeleteCompressionLayer( bool inputLayer ) override;
		
		/// \ingroup RAKNET_RPC
		/// Register a C or static member function as available for calling as a remote procedure call
		/// \param[in] uniqueID: A null-terminated unique string to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions
		/// \param[in] functionPointer(...): The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered
		void RegisterAsRemoteProcedureCall( RPCID  uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms, void* extra ), void* extra ) override;

		/// \ingroup RAKNET_RPC
		/// Register a C++ member function as available for calling as a remote procedure call.
		/// \param[in] uniqueID: A null terminated string to identify this procedure.Recommended you use the macro REGISTER_CLASS_MEMBER_RPC
		/// \param[in] functionPointer: The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered with UnregisterAsRemoteProcedureCall
		/// \sa ObjectMemberRPC.cpp
		void RegisterClassMemberRPC( RPCID  uniqueID, void *functionPointer ) override;
		
		/// \ingroup RAKNET_RPC
		/// Unregisters a C function as available for calling as a remote procedure call that was formerly registeredwith RegisterAsRemoteProcedureCallOnly call offline
		/// \param[in] uniqueID A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameterpassed to RegisterAsRemoteProcedureCall
		void UnregisterAsRemoteProcedureCall( RPCID  uniqueID ) override;
		
		/// \ingroup RAKNET_RPC
		/// Calls a C function on the remote system that was already registered using RegisterAsRemoteProcedureCall.
		/// If you want that function to return data you should call RPC from that system in the same wayReturns true on a successful packet
		/// send (this does not indicate the recipient performed the call), false on failure
		/// \param[in] uniqueID A NULL terimianted string to this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameter
		/// \param[in] data The data to send
		/// \param[in] bitLength The number of bits of \a data
		/// \param[in] priority What priority level to send on.
		/// \param[in] reliability How reliability to send this data
		/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure\note This is part of the Remote Procedure Call Subsystem 
		bool RPC( RPCID  uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget ) override;
		
		/// \ingroup RAKNET_RPC
		/// Calls a C function on the remote system that was already registered using RegisterAsRemoteProcedureCall.
		/// If you want that function to return data you should call RPC from that system in the same wayReturns true on a successful packet
		/// send (this does not indicate the recipient performed the call), false on failure
		/// \param[in] uniqueID A NULL terimianted string to this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameter
		/// \param[in] bitStream The bitstream to send
		/// \param[in] priority What priority level to send on.
		/// \param[in] reliability How reliability to send this data
		/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure\note This is part of the Remote Procedure Call Subsystem 
		bool RPC( RPCID  uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget ) override;

		/// Enables or disables frequency table tracking.  This is required to get a frequency table, which is used in GenerateCompressionLayer()
		/// This value persists between connect calls and defaults to false (no frequency tracking)
		/// \pre You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only trackpart of the values sent over the network.
		/// \param[in] b True to enable tracking 
		void SetTrackFrequencyTable( bool b ) override;
		
		/// Returns the frequency of outgoing bytes into outputFrequencyTable.  This is required to get a frequency table, which is used in GenerateCompressionLayer()
		/// The purpose is to save to file as either a master frequency table from a sample game session.
		/// \pre You should only call this when disconnected
		/// \pre Requires that you first enable data frequency tracking by calling SetTrackFrequencyTable(true)
		/// \param[out] outputFrequencyTable The Frequency Table used in the compression layer 
		/// \return false (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)	
		bool GetSendFrequencyTable( unsigned int outputFrequencyTable[ 256 ] ) override;
		
		/// Returns the compression ratio. A low compression ratio is good.  Compression is for outgoing data
		/// \return The compression ratio
		float GetCompressionRatio( void ) const override;

		///Returns the decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
		/// \return The decompression ratio
		float GetDecompressionRatio( void ) const override;

		/// Attatches a Plugin interface to run code automatically on message receipt in the Receive call
		/// \note If plugins have dependencies on each other then the order does matter - for example the router plugin should go first because it might route messages for other plugins
		/// \param[in] messageHandler Pointer to a message handler to attach
		void AttachPlugin( PluginInterface *messageHandler ) override;

		///Detaches a Plugin interface to run code automatically on message receipt
		/// \param[in] messageHandler Pointer to a message handler to detach
		void DetachPlugin( PluginInterface *messageHandler ) override;
		
		/// The server internally maintains a data struct that is
		/// automatically sent to clients when the connect. This is useful
		/// to contain data such as the server name or message of the day.
		/// Access that struct with this function.  The data is entered as an
		/// array and stored and returned as a BitStream.  Everytime you call
		/// GetStaticServerData it resets the read pointer to the start of
		/// the bitstream.  To do multiple reads without reseting the pointer
		/// Maintain a pointer copy to the bitstream as in RakNet::BitStream *copy = ...->GetStaticServerData(...);
		/// To store a bitstream, use the GetData() and GetNumberOfBytesUsed() methods
		/// of the bitstream for the 2nd and 3rd parameters
		/// Note that the server may change at any time the
		/// data contents and/or its length!
		/// \return a bitstream containing static server data
		RakNet::BitStream * GetStaticServerData( void ) override;
		
		/// The server internally maintains a data struct that is
		/// automatically sent to clients when the connect. This is useful
		/// to contain data such as the server name or message of the day.
		/// Access that struct with this function.  The data is entered as an
		/// array and stored and returned as a BitStream.  Everytime you call
		/// GetStaticServerData it resets the read pointer to the start of
		/// the bitstream.  To do multiple reads without reseting the pointer
		/// Maintain a pointer copy to the bitstream as in RakNet::BitStream *copy = ...->GetStaticServerData(...);
		/// To store a bitstream, use the GetData() and GetNumberOfBytesUsed() methods
		/// of the bitstream for the 2nd and 3rd parameters
		/// Note that the server may change at any time the
		/// data contents and/or its length!
		void SetStaticServerData( const char *data, const int length ) override;
		
		/// The client internally maintains a data struct that is automatically sent to the server on connection
		/// This is useful to contain data such as the player name. Access that struct with this
		/// function. Pass UNASSIGNED_PLAYER_ID for playerId to reference your internal data.  A playerId value to access the data of another player.
		/// *** NOTE ** * If you change any data in the struct the server won't reflect this change unless you manually update it
		/// Do so by calling SendStaticClientDataToServer
		/// The data is entered as an array and stored and returned as a BitStream.
		/// Everytime you call GetStaticServerData it resets the read pointer to the start of the bitstream.  To do multiple reads without reseting the pointer
		/// Maintain a pointer copy to the bitstream as in
		/// RakNet::BitStream *copy = ...->GetStaticServerData(...);
		/// To store a bitstream, use the GetData() and GetNumberOfBytesUsed() methods
		/// of the bitstream for the 2nd and 3rd parameters
		RakNet::BitStream * GetStaticClientData( const PlayerID playerId ) override;
		
		/// Set Local statistical information for playId. Call this
		/// function when you receive statistical information from a
		/// client.
		/// \param[in] playerId the player ID 
		/// \param[in] data the packet data 
		/// \param[in] length the size of the data 
		void SetStaticClientData( const PlayerID playerId, const char *data, const int length ) override;
		
		/// Send the static server data to the server The only time you need
		/// to call this function is to update clients that are already
		/// connected when you change the static server data by calling
		/// GetStaticServerData and directly modifying the object pointed to.
		/// Obviously if the connected clients don't need to know the new
		/// data you don't need to update them, so it's up to you The server
		/// must be active for this to have meaning
		void SendStaticClientDataToServer( void ) override;
		
		/// Return the player number of the server.
		/// \return the server playerID 
		PlayerID GetServerID( void ) const override;
		
		/// Return the player number the server has assigned to you.
		/// \return our player ID 
		///  \note that unlike in previous versions, this is a struct and is not sequential
		PlayerID GetPlayerID( void ) const override;

		///Return the unique address identifier that represents you on the the network and is based on your local IP / portNote that unlike in previous versions, this is a struct and is not sequential
		PlayerID GetInternalID( void ) const override;
		
		/// Returns the dotted IP address for the specified playerId
		/// \param[in] playerId Any player ID other than UNASSIGNED_PLAYER_ID, even if that player is not currently connected
		/// \return a dotted notation string representation of the address of playerId. 
		const char* PlayerIDToDottedIP( const PlayerID playerId ) const override;
		
		/// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
		/// \param[in] packet the packet to delayed 
		/// \pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
		void PushBackPacket( Packet *packet, bool pushAtHead ) override;

		/// \Internal
		void SetRouterInterface( RouterInterface *routerInterface ) override;

		/// \Internal
		void RemoveRouterInterface( RouterInterface *routerInterface ) override;

		/// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable packet
		/// Default time is 10,000 or 10 seconds in release and 30,000 or 30 seconds in debug.
		/// \param[in] timeMS Time, in MS
		void SetTimeoutTime( RakNetTime timeMS ) override;
		
		/// Set the MTU per datagram.  It's important to set this correctly - otherwise packets will be needlessly split, decreasing performance and throughput.
		/// Maximum allowed size is MAXIMUM_MTU_SIZE.
		/// Too high of a value will cause packets not to arrive at worst and be fragmented at best.
		/// Too low of a value will split packets unnecessarily.
		/// sa MTUSize.h
		/// \pre Can only be called when not connected.
		/// \return false on failure (we are connected), else true
		bool SetMTUSize( int size ) override;

		/// Returns the current MTU size
		/// \return The current MTU size
		int GetMTUSize( void ) const override;
		
		///Allow or disallow connection responses from any IP. Normally this should be false, but may be necessary
		/// when connection to servers with multiple IP addresses.
		/// \param[in] allow - True to allow this behavior, false to not allow. Defaults to false. Value persists between connections
		void AllowConnectionResponseIPMigration( bool allow ) override;
		
		/// Sends a one byte message ID_ADVERTISE_SYSTEM to the remote unconnected system.
		/// This will tell the remote system our external IP outside the LAN along with some user data.
		/// \pre The sender and recipient must already be started via a successful call to Initialize
		/// \param[in] host Either a dotted IP address or a domain name
		/// \param[in] remotePort Which port to connect to on the remote machine.
		/// \param[in] data Optional data to append to the packet.
		/// \param[in] dataLength length of data in bytes.  Use 0 if no data.
		void AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength ) override;
		
		/// Returns a structure containing a large set of network statistics for server
		/// You can map this data to a string using the C style StatisticsToString() function
		/// \sa RakNetStatistics.h
		RakNetStatisticsStruct * GetStatistics( void ) override;

		/// @internal 
		/// Retrieve the player index corresponding to this client. 
		PlayerIndex GetPlayerIndex( void ) override;
		
	private:

		
		int GetOtherClientIndexByPlayerID( const PlayerID playerId );
		int GetFreeOtherClientIndex( void );
		RakNet::BitStream password;
		
		struct OtherClientsStruct
		{
			///The id of the other player 
			
			PlayerID playerId;
			///The average ping time 
			
			short ping;
			///Other client's Static Client Data
			
			RakNet::BitStream staticData;
			///Tel whether the remote client is active or not 
			
			bool isActive;
		}
		
		otherClients[ 32 ];
		unsigned int seed;
		unsigned int nextSeed;
		RakNetTime nextSeedUpdate;
		PlayerIndex localPlayerIndex;
	};
}

#pragma clang diagnostic pop

#endif
