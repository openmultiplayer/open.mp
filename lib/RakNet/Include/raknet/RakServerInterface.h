/// \file
/// \brief An interface for RakServer.  Simply contains all user functions as pure virtuals.
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

#ifndef __RAK_SERVER_INTERFACE_H
#define __RAK_SERVER_INTERFACE_H

#include "NetworkTypes.h"
#include "PacketPriority.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "RakNetStatistics.h"
#include "RakPeer.h"
#include "Export.h"

namespace RakNet
{
	/// This is a user-interface class to act as a game server.  All it does is implement some functionality on top of RakPeer.
	/// See the individual functions for what the class can do.
	/// \brief Defines the functions used by a game server
	class RAK_DLL_EXPORT RakServerInterface
	{

	public:
			///Destructor
		virtual ~RakServerInterface() {}

		/// Call this to initiate the server with the number of players you want to be allowed connected at once
		/// \param[in] AllowedPlayers Current maximum number of allowed players is 65535
		/// \param[in] depreciated is for legacy purposes and is unused
		/// \param[in] threadSleepTimer How many ms to Sleep each internal update cycle (30 to give the game priority, 0 for regular (recommended), -1 to not Sleep() (may be slower))
		/// \param[in] port is the port you want the server to read and write onMake sure this port is open for UDP
		/// \param[in] forceHostAddress Can force RakNet to use a particular IP to host on.  Pass 0 to automatically pick an IP
		/// \return true on successful initiation, false otherwise
		virtual bool Start( unsigned short AllowedPlayers, unsigned int depreciated, int threadSleepTimer, unsigned short port, const char *forceHostAddress=0 )=0;
		
		/// Must be called while offlineSecures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to preventconnection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.There is a significant amount of processing and a slight amount of bandwidthoverhead for this feature.If you accept connections, you must call this or else secure connections will not be enabledfor incoming connections. If the private keys are 0, then a new key will be generated when this function is called@see the Encryption sample
		/// \param[in] privateKeyE A pointer to the public keys from the RSACrypt class.  
		/// \param[in] privateKeyN A pointer to the public keys from the RSACrypt class. 
		virtual void InitializeSecurity( const char *privateKeyE, const char *privateKeyN )=0;

		/// Disables all security.
		/// \pre Must be called while offline
		virtual void DisableSecurity( void )=0;
		
		/// Set the password clients have to use to connect to this server. The password persists between connections.
		/// Pass 0 for no password.  You can call this anytime
		/// \param[in] _password The password string, or 0 for none.
		virtual void SetPassword( const char *_password )=0;
		
		/// Returns if non-zero was passed to SetPassword()
		// \return true if a password was set, false if not.
		virtual bool HasPassword( void )=0;
		
		/// Stops the server, stops synchronized data, and resets all internal data.  This will drop all players currently connected, howeversince the server is stopped packet reliability is not enforced so the Kick network message may not actuallyarrive. Those players will disconnect due to timeout. If you want to end the server more gracefully, youcan manually Kick each player first. Does nothing if the server is not running to begin with
		/// \param[in] blockDuration The maximum amount of time to wait for all remaining packets to go out, including the disconnection notification. If you set it to 0 then the disconnection notifications probably won't arrive
		/// \param[in] orderingChannel If blockDuration > 0, the disconnect packet will be sent on this channel
		virtual void Disconnect( unsigned int blockDuration, unsigned char orderingChannel=0 )=0;
		
		/// /pre The server must be active.
		/// Send the data stream of length \a length to whichever \a playerId you specify.
		/// \param[in] data The data to send
		/// \param[in] length The length, in bytes, of \a data
		/// \param[in] priority See PacketPriority
		/// \param[in] reliability See PacketReliabilty
		/// \param[in] orderingChannel The ordering channel to use, from 0 to 31.  Ordered or sequenced packets sent on the channel arrive ordered or sequence in relation to each other.  See the manual for more details on this.
		/// \param[in] playerId Who to send to.  Specify UNASSIGNED_PLAYER_ID to designate all connected systems.
		/// \param[in] broadcast Whether to send to everyone or not.  If true, then the meaning of \a playerId changes to mean who NOT to send to.
		/// \return Returns false on failure, true on success	
		virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )=0;
		
		/// /pre The server must be active.
		/// Send the data stream of length \a length to whichever \a playerId you specify.
		/// \param[in] bitStream The bitstream to send.
		/// \param[in] priority See PacketPriority
		/// \param[in] reliability See PacketReliabilty
		/// \param[in] orderingChannel The ordering channel to use, from 0 to 31.  Ordered or sequenced packets sent on the channel arrive ordered or sequence in relation to each other.  See the manual for more details on this.
		/// \param[in] playerId Who to send to.  Specify UNASSIGNED_PLAYER_ID to designate all connected systems.
		/// \param[in] broadcast Whether to send to everyone or not.  If true, then the meaning of \a playerId changes to mean who NOT to send to.
		/// \return Returns false on failure, true on success	
		virtual bool Send( RakNet::BitStream const *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )=0;
		
		/// Gets a packet from the incoming packet queue.
		/// Use DeallocatePacket() to deallocate the packet after you are done with it.
		/// User-thread functions, such as RPC calls and the plugin function PluginInterface::Update occur here.
		/// \return 0 if no packets are waiting to be handled, otherwise a pointer to a packet.
		// sa CoreNetworkStructures.h contains struct Packet
		virtual Packet* Receive( void )=0;
		
		/// Kick out the specified player.
		/// \param[in] playerId The playerId of the player to kcik.
		virtual void Kick( const PlayerID playerId )=0;
		
		/// Call this to deallocate a packet returned by Receive when you are done handling it.
		/// \param[in] packet The packet to deallocate.	
		virtual void DeallocatePacket( Packet *packet )=0;
		
		/// Set how many players are allowed on the server.
		/// If more players are currently connected then are allowed then no more players will be allowed to join until the number of players is less than the number of allowed players.
		/// \pre The server must be active for this to have meaning
		/// \param[in] AllowedPlayers The number of players to allow
		virtual void SetAllowedPlayers( unsigned short AllowedPlayers )=0;
		
		/// Return how many players are allowed to connect. This value was set either from Start or from SetAllowedPlayers.
		/// \pre The server must be active for this to have meaning
		/// \return The number of allowed players
		virtual unsigned short GetAllowedPlayers( void ) const=0;
		
		///Return how many players are currently connected to the server.
		/// \pre The server must be active for this to have meaning
		/// \return The number of connected players.
		virtual unsigned short GetConnectedPlayers( void )=0;
		
		/// Returns a static string pointer containing the IP of the specified connected player.
		/// Also returns the client port.
		/// This changes between calls so be sure to copy what is returned if you need to retain it.
		/// Useful for creating and maintaining ban lists.
		/// \pre The server must be active for this to have meaning
		/// If the specified id does not represent an active player the results are undefined (most likely returns 0)
		/// \param[in] playerId Which player we are referring to
		/// \param[out] returnValue The IP of this player we are referring to
		/// \param[out] port The port of the player we are referring to.
		virtual void GetPlayerIPFromID( const PlayerID playerId, char returnValue[ 22 ], unsigned short *port )=0;
		
		/// Send a ping request to the specified player
		/// \param[in] playerId Which player we are referring to
		virtual void PingPlayer( const PlayerID playerId )=0;
		
		///Returns the average of all ping times read for the specific player or -1 if none read yet
		/// \param[in] playerId Which player we are referring to
		/// \return The ping time for this player, or -1
		virtual int GetAveragePing( const PlayerID playerId )=0;
		
		/// Returns the last ping time read for the specific player or -1 if none read yet
		/// \param[in] playerId Which player we are referring to
		/// \return The last ping time for this player, or -1
		virtual int GetLastPing( const PlayerID playerId )=0;
		
		/// Returns the lowest ping time read or -1 if none read yet
		/// \param[in] playerId Which player we are referring to
		/// \return The lowest ping time for this player, or -1
		virtual int GetLowestPing( const PlayerID playerId )=0;
		
		/// Ping all players every so often.  This is on by default.  In games where you don't care about ping you can callStopOccasionalPing to save the bandwidth.
		/// This can be called anytime.
		virtual void StartOccasionalPing( void )=0;
		
		/// Stop pinging players every so often.  Players are pinged by default.  In games where you don't care about ping you can call this to save the bandwidthThis will work anytime
		virtual void StopOccasionalPing( void )=0;
		
		/// Returns true if the server is currently active
		virtual bool IsActive( void ) const=0;
		
		/// Returns a number automatically synchronized between the server and client which randomly changes every 9 seconds.
		/// The time it changes is accurate to within a few ms and is best used to seed random number generators that you want to usually
		/// return the same output on all systems.
		/// Keep in mind thisisn't perfectly accurate as there is always a very small chance the numbers will by out of synch.
		/// You should should confine its use to visual effects or functionality that has a backup method to maintain synchronization.
		/// If you don't need this functionality and want to save the bandwidth callStopSynchronizedRandomInteger after starting the server
		/// \return A number, which is probably synchronized among all clients and the server.
		virtual unsigned int GetSynchronizedRandomInteger( void ) const=0;
		
		/// Start or restart the synchronized random integer.  This is on by default.  Call StopSynchronizedRandomIntegerto stop it keeping the number in synch
		virtual void StartSynchronizedRandomInteger( void )=0;
		
		/// Stop the synchronized random integer.  Call StartSynchronizedRandomInteger to start it again
		virtual void StopSynchronizedRandomInteger( void )=0;

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
		virtual bool GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )=0;
		
		/// Delete the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory.
		/// \pre You should only call this when disconnected
		/// \param[in] inputLayer True to mean the inputLayer, false to mean the output layer
		/// \return false (failure) if connected.  Otherwise true (success)
		virtual bool DeleteCompressionLayer( bool inputLayer )=0;
		
		/// \ingroup RAKNET_RPC
		/// Register a C or static member function as available for calling as a remote procedure call
		/// \param[in] uniqueID: A null-terminated unique string to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions
		/// \param[in] functionPointer  The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered
		virtual void RegisterAsRemoteProcedureCall( RPCID uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms, void* extra ), void* extra )=0;

		/// \ingroup RAKNET_RPC
		/// Register a C++ member function as available for calling as a remote procedure call.
		/// \param[in] uniqueID: A null terminated string to identify this procedure.Recommended you use the macro REGISTER_CLASS_MEMBER_RPC
		/// \param[in] functionPointer: The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered with UnregisterAsRemoteProcedureCall
		/// \sa ObjectMemberRPC.cpp
		virtual void RegisterClassMemberRPC( RPCID uniqueID, void *functionPointer )=0;

		///\ingroup RAKNET_RPC
		/// Unregisters a C function as available for calling as a remote procedure call that was formerly registeredwith RegisterAsRemoteProcedureCallOnly call offline
		/// \param[in] uniqueID A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameterpassed to RegisterAsRemoteProcedureCall
		virtual void UnregisterAsRemoteProcedureCall( RPCID uniqueID )=0;

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
		/// \param[in] playerId Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure\note This is part of the Remote Procedure Call Subsystem 
		virtual bool RPC( RPCID uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )=0;

		/// \ingroup RAKNET_RPC
		/// Calls a C function on the remote system that was already registered using RegisterAsRemoteProcedureCall.
		/// If you want that function to return data you should call RPC from that system in the same wayReturns true on a successful packet
		/// send (this does not indicate the recipient performed the call), false on failure
		/// \param[in] uniqueID A NULL terimianted string to this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameter
		/// \param[in] bitStream The bitstream to send
		/// \param[in] priority What priority level to send on.
		/// \param[in] reliability How reliability to send this data
		/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.
		/// \param[in] playerId Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure
		virtual bool RPC( RPCID uniqueID, RakNet::BitStream const *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )=0;

		/// Enables or disables frequency table tracking.  This is required to get a frequency table, which is used in GenerateCompressionLayer()
		/// This value persists between connect calls and defaults to false (no frequency tracking)
		/// \pre You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only trackpart of the values sent over the network.
		/// \param[in] b True to enable tracking 
		virtual void SetTrackFrequencyTable( bool b )=0;
		
		/// Returns the frequency of outgoing bytes into outputFrequencyTable.  This is required to get a frequency table, which is used in GenerateCompressionLayer()
		/// The purpose is to save to file as either a master frequency table from a sample game session.
		/// \pre You should only call this when disconnected
		/// \pre Requires that you first enable data frequency tracking by calling SetTrackFrequencyTable(true)
		/// \param[out] outputFrequencyTable The Frequency Table used in the compression layer 
		/// \return false (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
		virtual bool GetSendFrequencyTable( unsigned int outputFrequencyTable[ 256 ] )=0;
		
		/// Returns the compression ratio. A low compression ratio is good.  Compression is for outgoing data
		/// \return The compression ratio
		virtual float GetCompressionRatio( void ) const=0;
		
		///Returns the decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
		/// \return The decompression ratio
		virtual float GetDecompressionRatio( void ) const=0;
		
		/// Attatches a Plugin interface to run code automatically on message receipt in the Receive call
		/// \note If plugins have dependencies on each other then the order does matter - for example the router plugin should go first because it might route messages for other plugins
		/// \param[in] messageHandler Pointer to a message handler to attach
		virtual void AttachPlugin( PluginInterface *messageHandler )=0;

		///Detaches a Plugin interface to run code automatically on message receipt
		/// \param[in] messageHandler Pointer to a message handler to detach
		virtual void DetachPlugin( PluginInterface *messageHandler )=0;
		
		///The server internally maintains a data struct that is automatically sent to clients when they connect.
		/// This is useful to contain data such as the server name or message of the day.
		/// Access that struct with thisfunction.
		/// \note If you change any data in the struct remote clients won't reflect this change unless you manually update themDo so by calling SendStaticServerDataToClient(UNASSIGNED_PLAYER_ID)
		/// \return The static server data, stored in a bitstream
		virtual RakNet::BitStream * GetStaticServerData( void )=0;
		
		/// The server internally maintains a data struct that is automatically sent to clients when they connect.
		/// This function will set that data.
		/// \param[in] data The data to use as the static server data
		/// \param[in] length The length, in bytes, of \a data
		virtual void SetStaticServerData( const char *data, const int length )=0;
		
		/// This sets to true or false whether we want to support relaying of static client data to other connected clients.
		/// If set to false it saves some bandwdith, however other clients won't know the static client data and attempting to read that data will return garbage.
		/// Default is false.  This only works for up to 32 players.  Games supporting more than 32 players will have this shut off automatically upon server start and must have it forced back on with this function
		/// if you do indeed want it.
		/// \pre This should be called after the server is started in case you want to override when it shuts off at 32 players
		/// \param[in] b true or false, as you wish to set the function
		virtual void SetRelayStaticClientData( bool b )=0;
		
		/// Send the static server data to the specified player.
		/// Pass UNASSIGNED_PLAYER_ID to broadcast to all players.
		/// The only time you need to call this function is to update clients that are already connected when you change the static server data by calling GetStaticServerData() and directly modifying the object pointed to.
		/// Obviously if theconnected clients don't need to know the new data you don't need to update them, so it's up to you.
		/// \pre The server must be active for this to have meaning
		/// \param[in] playerId The playerId we are referring to
		virtual void SendStaticServerDataToClient( const PlayerID playerId )=0;

		/// Sets the data to send along with a LAN server discovery or offline ping reply.
		/// \a length should be under 400 bytes, as a security measure against flood attacks
		/// \param[in] data a block of data to store, or 0 for none
		/// \param[in] length The length of data in bytes, or 0 for none
		/// \sa Ping.cpp
		virtual void SetOfflinePingResponse( const char *data, const unsigned int length )=0;
		
		/// Returns a pointer to an attached client's static data specified by the playerId.
		/// Returns 0 if no such player is connected.
		/// \note that you can modify the client data here.  Changes won't be reflected on clients unless you force them to update by calling ChangeStaticClientData()
		/// \pre The server must be active for this to have meaning
		/// \param[in] playerId The ID of the client 
		/// \return A bitstream containing the static client data
		virtual RakNet::BitStream * GetStaticClientData( const PlayerID playerId )=0;
		
		/// Set the stait client data for a particular player
		/// \param[in] playerId The ID of the client 
		/// \param[in] data a block of data to store, or 0 for none
		/// \param[in] length The length of data in bytes, or 0 for none
		virtual void SetStaticClientData( const PlayerID playerId, const char *data, const int length )=0;
		
		/// This function is used to update the information on connected clients when the server effects a change of static client data.
		/// Note that a client that gets this new information for himself will update the data for his playerID but not his local data (which is the user's copy) i.e. player 5 would have the data returned by GetStaticClientData(5) changed but his local information returned byGetStaticClientData(UNASSIGNED_PLAYER_ID) would remain the same as it was previously.
		/// \param[in] playerChangedId The playerId of the player whose data was changed.  This is the parameter passed toGetStaticClientData to get a pointer to that player's information.
		/// \param[in] playerToSendToId The player you want to update with the new information.  This will almost always be everybody, in which case you should pass UNASSIGNED_PLAYER_ID.
		/// \pre The server must be active for this to have meaning
		virtual void ChangeStaticClientData( const PlayerID playerChangedId, PlayerID playerToSendToId )=0;
		
		/// Internally store the IP address(es) for the server and return how many it has.
		/// This can easily be more than one, for example a system on both a LAN and with a net connection.
		/// The server does not have to be active for this to work
		virtual unsigned int GetNumberOfAddresses( void )=0;
		
		/// Call this function where 0 <= index < x where x is the value returned by GetNumberOfAddresses()
		/// Returns a static string filled with the server IP of the specified index.
		/// Strings returned in no particular order.  You'll have to check every index see which string you want.
		/// \return  0 on invalid input, otherwise an dotted IP
		virtual const char* GetLocalIP( unsigned int index )=0;

		/// Return the unique address identifier that represents you on the the network and is based on your local IP / port.
		/// \note Unlike in previous versions, PlayerID is a struct and is not sequential
		virtual PlayerID GetInternalID( void ) const=0;

		/// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately.
		/// \param[in] packet The packet to push
		/// \param[in] pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
		virtual void PushBackPacket( Packet *packet, bool pushAtHead )=0;

		/// \Internal
		virtual void SetRouterInterface( RouterInterface *routerInterface )=0;

		/// \Internal
		virtual void RemoveRouterInterface( RouterInterface *routerInterface )=0;
		
		/// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
		/// \param[in] playerId The PlayerID we are referring to
		/// \return The index of this PlayerID
		virtual int GetIndexFromPlayerID( const PlayerID playerId )=0;
		
		/// This function is only useful for looping through all players.
		/// Given an index, will return a PlayerID.
		/// \param[in] index Index should range between 0 and the maximum number of players allowed - 1.
		virtual PlayerID GetPlayerIDFromIndex( int index )=0;

		/// Bans an IP from connecting.  Banned IPs persist between connections.
		/// param[in] IP Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will banAll IP addresses starting with 128.0.0
		virtual void AddToBanList( const char *IP, RakNetTime milliseconds=0 )=0;

		/// Allows a previously banned IP to connect. 
		/// param[in] Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will banAll IP addresses starting with 128.0.0
		virtual void RemoveFromBanList( const char *IP )=0;

		/// Allows all previously banned IPs to connect.
		virtual void ClearBanList( void )=0;

		/// Returns true or false indicating if a particular IP is banned.
		/// \param[in] IP - Dotted IP address.
		/// \return true if IP matches any IPs in the ban list, accounting for any wildcards. false otherwise.
		virtual bool IsBanned( const char *IP )=0;
		
		/// Returns true if that player ID is currently active
		/// \param[in] playerId Which playerId we are referring to
		/// \return true or false, as noted
		virtual bool IsActivePlayerID( const PlayerID playerId )=0;

		/// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable packet
		/// Default time is 10,000 or 10 seconds in release and 30,000 or 30 seconds in debug.
		/// \param[in] timeMS Time, in MS
		/// \param[in] target Which system to do this for
		virtual void SetTimeoutTime( RakNetTime timeMS, const PlayerID target )=0;
		
		/// Set the MTU per datagram.  It's important to set this correctly - otherwise packets will be needlessly split, decreasing performance and throughput.
		/// Maximum allowed size is MAXIMUM_MTU_SIZE.
		/// Too high of a value will cause packets not to arrive at worst and be fragmented at best.
		/// Too low of a value will split packets unnecessarily.
		/// sa MTUSize.h
		/// \pre Can only be called when not connected.
		/// \return false on failure (we are connected), else true
		virtual bool SetMTUSize( int size )=0;
		
		/// Returns the current MTU size
		/// \return The current MTU size
		virtual int GetMTUSize( void ) const=0;

		/// Sends a one byte message ID_ADVERTISE_SYSTEM to the remote unconnected system.
		/// This will tell the remote system our external IP outside the LAN along with some user data..
		/// \param[in] host Either a dotted IP address or a domain name
		/// \param[in] remotePort Which port to connect to on the remote machine.
		/// \param[in] data to send along with ID_ADVERTISE_SYSTEM
		/// \param[in] dataLength The length of \a data
		virtual void AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength )=0;
		
		/// Returns a structure containing a large set of network statistics for the specified system.
		/// You can map this data to a string using the C style StatisticsToString() function
		/// \param[in] playerId: Which connected system to get statistics for
		/// \return 0 on can't find the specified system.  A pointer to a set of data otherwise.
		/// \sa RakNetStatistics.h
		virtual RakNetStatisticsStruct * GetStatistics( const PlayerID playerId )=0;

		/// Get SAMP data for a player from their ID
		virtual SAMPRakNet::RemoteSystemData GetSAMPDataFromPlayerID(const PlayerID playerId) = 0;

		/// Get Remote System data for a player from their ID
		virtual RakPeer::RemoteSystemStruct* GetRemoteSystemFromPlayerID(const PlayerID playerId) = 0;
	};
}

#endif
