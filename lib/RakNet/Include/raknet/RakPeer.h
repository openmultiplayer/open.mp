/// \file
/// \brief The main class used for data transmission and most of RakNet's functionality.
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

#ifndef __RAK_PEER_H
#define __RAK_PEER_H

#include "Export.h"
#include "RakPeerInterface.h"
#include "ReliabilityLayer.h"
#include "RPCNode.h"
#include "RSACrypt.h"
#include "BitStream.h"
#include "SingleProducerConsumer.h"
#include "RPCMap.h"
#include "SimpleMutex.h"
#include "DS_OrderedList.h"

namespace RakNet
{
	namespace DataStructures
	{
		class HuffmanEncodingTree;
	}

	class PluginInterface;

	// Sucks but this struct has to be outside the class.  Inside and DevCPP won't let you refer to the struct as RakPeer::PlayerIDAndIndex while GCC
	// forces you to do RakPeer::PlayerIDAndIndex
	struct PlayerIDAndIndex{PlayerID playerId;unsigned index;};
	int RAK_DLL_EXPORT PlayerIDAndIndexComp( const PlayerID &key, const PlayerIDAndIndex &data ); // GCC requires RakPeer::PlayerIDAndIndex or it won't compile

	#ifdef _WIN32
	// friend unsigned __stdcall RecvFromNetworkLoop(LPVOID arguments);
	void __stdcall ProcessPortUnreachable(const unsigned int binaryAddress, const unsigned short port, RakPeer *rakPeer);
	void __stdcall ProcessNetworkPacket(const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer);
	unsigned __stdcall UpdateNetworkLoop(LPVOID arguments);
	#else
	// friend void*  RecvFromNetworkLoop( void*  arguments );
	void ProcessPortUnreachable(const unsigned int binaryAddress, const unsigned short port, RakPeer *rakPeer);
	void ProcessNetworkPacket(const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer);
	void* UpdateNetworkLoop(void* arguments);
	#endif

	/// The primary interface for RakNet, RakPeer contains all major functions for the library.
	/// See the individual functions for what the class can do.
	/// \brief The main interface for network communications
	class RAK_DLL_EXPORT RakPeer : public RakPeerInterface
	{
	public:
		///Constructor
		RakPeer();

		///Destructor
		virtual ~RakPeer();

		// --------------------------------------------------------------------------------------------Major Low Level Functions - Functions needed by most users--------------------------------------------------------------------------------------------
		/// \brief Starts the network threads, opens the listen port.
		/// You must call this before calling Connect().
		/// Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Disconnect().
		/// \note Call SetMaximumIncomingConnections if you want to accept incoming connections
		/// \param[in] maxConnections The maximum number of connections between this instance of RakPeer and another instance of RakPeer. Required so the network can preallocate and for thread safety. A pure client would set this to 1.  A pure server would set it to the number of allowed clients.- A hybrid would set it to the sum of both types of connections
		/// \param[in] localPort The port to listen for connections on.
		/// \param[in] _threadSleepTimer How many ms to Sleep each internal update cycle (30 to give the game priority, 0 for regular (recommended), -1 to not Sleep() (may be slower))
		/// \param[in] forceHostAddress Can force RakNet to use a particular IP to host on.  Pass 0 to automatically pick an IP
		/// \return False on failure (can't create socket or thread), true on success.
		bool Initialize( unsigned short maxConnections, unsigned short localPort, int _threadSleepTimer, const char *forceHostAddress=0 ) override;

		/// Secures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to prevent connection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.
		/// There is a significant amount of processing and a slight amount of bandwidth overhead for this feature.
		/// If you accept connections, you must call this or else secure connections will not be enabled for incoming connections.
		/// If you are connecting to another system, you can call this with values for the (e and p,q) public keys before connecting to prevent MitM
		/// \pre Must be called before Initialize
		/// \param[in] pubKeyE A pointer to the public keys from the RSACrypt class.  
		/// \param[in] pubKeyN A pointer to the public keys from the RSACrypt class. 
		/// \param[in] privKeyP Public key generated from the RSACrypt class.  
		/// \param[in] privKeyQ Public key generated from the RSACrypt class.  If the private keys are 0, then a new key will be generated when this function is called@see the Encryption sample
		void InitializeSecurity(const char *pubKeyE, const char *pubKeyN, const char *privKeyP, const char *privKeyQ ) override;

		/// Disables all security.
		/// \note Must be called while offline
		void DisableSecurity( void ) override;

		/// Sets how many incoming connections are allowed. If this is less than the number of players currently connected,
		/// no more players will be allowed to connect.  If this is greater than the maximum number of peers allowed,
		/// it will be reduced to the maximum number of peers allowed.  Defaults to 0.
		/// \param[in] numberAllowed Maximum number of incoming connections allowed.
		void SetMaximumIncomingConnections( unsigned short numberAllowed ) override;

		/// Returns the value passed to SetMaximumIncomingConnections()
		/// \return the maximum number of incoming connections, which is always <= maxConnections
		unsigned short GetMaximumIncomingConnections( void ) const override;

		/// Sets the password incoming connections must match in the call to Connect (defaults to none). Pass 0 to passwordData to specify no password
		/// This is a way to set a low level password for all incoming connections.  To selectively reject connections, implement your own scheme using CloseConnection() to remove unwanted connections
		/// \param[in] passwordData A data block that incoming connections must match.  This can be just a password, or can be a stream of data. Specify 0 for no password data
		/// \param[in] passwordDataLength The length in bytes of passwordData
		void SetIncomingPassword( const char* passwordData, int passwordDataLength ) override;

		/// Gets the password passed to SetIncomingPassword
		/// \param[out] passwordData  Should point to a block large enough to hold the password data you passed to SetIncomingPassword()
		/// \param[in,out] passwordDataLength Maximum size of the array passwordData.  Modified to hold the number of bytes actually written
		void GetIncomingPassword( char* passwordData, int *passwordDataLength  ) override;

		/// \brief Connect to the specified host (ip or domain name) and server port.
		/// Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.
		/// Calling both acts as a true peer. This is a non-blocking connection.
		/// You know the connection is successful when IsConnected() returns true or Receive() gets a message with the type identifier ID_CONNECTION_ACCEPTED.
		/// If the connection is not successful, such as a rejected connection or no response then neither of these things will happen.
		/// \pre Requires that you first call Initialize
		/// \param[in] host Either a dotted IP address or a domain name
		/// \param[in] remotePort Which port to connect to on the remote machine.
		/// \param[in] passwordData A data block that must match the data block on the server passed to SetIncomingPassword.  This can be a string or can be a stream of data.  Use 0 for no password.
		/// \param[in] passwordDataLength The length in bytes of passwordData
		/// \return True on successful initiation. False on incorrect parameters, internal error, or too many existing peers.  Returning true does not mean you connected!
		bool Connect( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength ) override;

		/// \brief Stops the network threads and closes all connections.
		/// \param[in] blockDuration How long you should wait for all remaining messages to go out, including ID_DISCONNECTION_NOTIFICATION.  If 0, it doesn't wait at all.
		/// \param[in] orderingChannel If blockDuration > 0, ID_DISCONNECTION_NOTIFICATION will be sent on this channel
		/// If you set it to 0 then the disconnection notification won't be sent
		void Disconnect( unsigned int blockDuration, unsigned char orderingChannel=0 ) override;

		/// Returns if the network thread is running
		/// \return true if the network thread is running, false otherwise
		bool IsActive( void ) const override;

		/// Fills the array remoteSystems with the SystemID of all the systems we are connected to
		/// \param[out] remoteSystems An array of PlayerID structures to be filled with the PlayerIDs of the systems we are connected to. Pass 0 to remoteSystems to only get the number of systems we are connected to
		/// \param[in, out] numberOfSystems As input, the size of remoteSystems array.  As output, the number of elements put into the array 
		bool GetConnectionList( PlayerID *remoteSystems, unsigned short *numberOfSystems ) const override;

		/// Sends a block of data to the specified system that you are connected to.
		/// This function only works while the connected
		/// \param[in] data The block of data to send
		/// \param[in] length The size in bytes of the data to send
		/// \param[in] priority What priority level to send on.  See PacketPriority.h
		/// \param[in] reliability How reliability to send this data.  See PacketPriority.h
		/// \param[in] orderingChannel When using ordered or sequenced messages, what channel to order these on. Messages are only ordered relative to other messages on the same stream
		/// \param[in] playerId Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \return False if we are not connected to the specified recipient.  True otherwise
		bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast ) override;

		/// Sends a block of data to the specified system that you are connected to.  Same as the above version, but takes a BitStream as input.
		/// \param[in] bitStream The bitstream to send
		/// \param[in] priority What priority level to send on.  See PacketPriority.h
		/// \param[in] reliability How reliability to send this data.  See PacketPriority.h
		/// \param[in] orderingChannel When using ordered or sequenced messages, what channel to order these on. Messages are only ordered relative to other messages on the same stream
		/// \param[in] playerId Who to send this packet to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \return False if we are not connected to the specified recipient.  True otherwise
		bool Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast ) override;

		/// Gets a message from the incoming message queue.
		/// Use DeallocatePacket() to deallocate the message after you are done with it.
		/// User-thread functions, such as RPC calls and the plugin function PluginInterface::Update occur here.
		/// \return 0 if no packets are waiting to be handled, otherwise a pointer to a packet.
		/// sa NetworkTypes.h contains struct Packet
		Packet* Receive( void ) override;

		/// Call this to deallocate a message returned by Receive() when you are done handling it.
		/// \param[in] packet The message to deallocate.	
		void DeallocatePacket( Packet *packet ) override;

		/// Return the total number of connections we are allowed
		unsigned short GetMaximumNumberOfPeers( void ) const override;

		// --------------------------------------------------------------------------------------------Remote Procedure Call Functions - Functions to initialize and perform RPC--------------------------------------------------------------------------------------------
		/// \ingroup RAKNET_RPC
		/// Register a C or static member function as available for calling as a remote procedure call
		/// \param[in] uniqueID A null-terminated unique string to identify this procedure.  See RegisterClassMemberRPC() for class member functions.
		/// \param[in] functionPointer(...) The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered
		void RegisterAsRemoteProcedureCall( RPCID  uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms, void* extra ), void* extra ) override;

		/// \ingroup RAKNET_RPC
		/// Register a C++ member function as available for calling as a remote procedure call.
		/// \param[in] uniqueID A null terminated string to identify this procedure. Recommended you use the macro REGISTER_CLASS_MEMBER_RPC to create the string.  Use RegisterAsRemoteProcedureCall() for static functions.
		/// \param[in] functionPointer The name of the function to be used as a function pointer. This can be called whether active or not, and registered functions stay registered unless unregistered with UnregisterAsRemoteProcedureCall
		/// \sa The sample ObjectMemberRPC.cpp
		void RegisterClassMemberRPC( RPCID  uniqueID, void *functionPointer ) override;

		/// \ingroup RAKNET_RPC
		/// Unregisters a C function as available for calling as a remote procedure call that was formerly registered with RegisterAsRemoteProcedureCall. Only call offline.
		/// \param[in] uniqueID A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.
		void UnregisterAsRemoteProcedureCall( RPCID  uniqueID ) override;

		/// \ingroup RAKNET_RPC
		/// Calls a C function on the remote system that was already registered using RegisterAsRemoteProcedureCall().
		/// \param[in] uniqueID A NULL terminated string identifying the function to call.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.
		/// \param[in] data The data to send
		/// \param[in] bitLength The number of bits of \a data
		/// \param[in] priority What priority level to send on. See PacketPriority.h.
		/// \param[in] reliability How reliability to send this data. See PacketPriority.h.
		/// \param[in] orderingChannel When using ordered or sequenced message, what channel to order these on.
		/// \param[in] playerId Who to send this message to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which should be remotely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure
		bool RPC( RPCID  uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget ) override;

		/// \ingroup RAKNET_RPC
		/// Calls a C function on the remote system that was already registered using RegisterAsRemoteProcedureCall.
		/// If you want that function to return data you should call RPC from that system in the same wayReturns true on a successful packet
		/// send (this does not indicate the recipient performed the call), false on failure
		/// \param[in] uniqueID A NULL terminated string identifying the function to call.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.
		/// \param[in] data The data to send
		/// \param[in] bitLength The number of bits of \a data
		/// \param[in] priority What priority level to send on. See PacketPriority.h.
		/// \param[in] reliability How reliability to send this data. See PacketPriority.h.
		/// \param[in] orderingChannel When using ordered or sequenced message, what channel to order these on.
		/// \param[in] playerId Who to send this message to, or in the case of broadcasting who not to send it to.  Use UNASSIGNED_PLAYER_ID to specify none
		/// \param[in] broadcast True to send this packet to all connected systems. If true, then playerId specifies who not to send the packet to.
		/// \param[in] shiftTimestamp True to add a timestamp to your data, such that the first byte is ID_TIMESTAMP and the next sizeof(RakNetTime) is the timestamp.
		/// \param[in] networkID For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
		/// \param[in] replyFromTarget If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which should be remotely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
		/// \return True on a successful packet send (this does not indicate the recipient performed the call), false on failure
		bool RPC( RPCID  uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget ) override;
		
		// -------------------------------------------------------------------------------------------- Connection Management Functions--------------------------------------------------------------------------------------------
		/// Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
		/// \param[in] target Which system to close the connection to.
		/// \param[in] sendDisconnectionNotification True to send ID_DISCONNECTION_NOTIFICATION to the recipient.  False to close it silently.
		/// \param[in] channel Which ordering channel to send the disconnection notification on, if any
		void CloseConnection( const PlayerID target, bool sendDisconnectionNotification, unsigned char orderingChannel=0 ) override;

		/// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
		/// \param[in] playerId The PlayerID we are referring to
		/// \return The index of this PlayerID or -1 on system not found.
		int GetIndexFromPlayerID( const PlayerID playerId ) override;

		/// This function is only useful for looping through all systems
		/// Given an index, will return a PlayerID.
		/// \param[in] index Index should range between 0 and the maximum number of players allowed - 1.
		/// \return The PlayerID
		PlayerID GetPlayerIDFromIndex( int index ) override;

		/// Bans an IP from connecting.  Banned IPs persist between connections but are not saved on shutdown nor loaded on startup.
		/// param[in] IP Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will ban all IP addresses starting with 128.0.0
		/// \param[in] milliseconds how many ms for a temporary ban.  Use 0 for a permanent ban
		void AddToBanList( const char *IP, RakNetTime milliseconds=0 ) override;

		/// Allows a previously banned IP to connect. 
		/// param[in] Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will banAll IP addresses starting with 128.0.0
		void RemoveFromBanList( const char *IP ) override;

		/// Allows all previously banned IPs to connect.
		void ClearBanList( void ) override;

		/// Returns true or false indicating if a particular IP is banned.
		/// \param[in] IP - Dotted IP address.
		/// \return true if IP matches any IPs in the ban list, accounting for any wildcards. False otherwise.
		bool IsBanned( const char *IP ) override;
		
		// --------------------------------------------------------------------------------------------Pinging Functions - Functions dealing with the automatic ping mechanism--------------------------------------------------------------------------------------------
		/// Send a ping to the specified connected system.
		/// \pre The sender and recipient must already be started via a successful call to Initialize()
		/// \param[in] target Which system to ping
		void Ping( const PlayerID target ) override;

		/// Send a ping to the specified unconnected system. The remote system, if it is Initialized, will respond with ID_PONG. The final ping time will be encoded in the following sizeof(RakNetTime) bytes.  (Default is 4 bytes - See __GET_TIME_64BIT in NetworkTypes.h
		/// \param[in] host Either a dotted IP address or a domain name.  Can be 255.255.255.255 for LAN broadcast.
		/// \param[in] remotePort Which port to connect to on the remote machine.
		/// \param[in] onlyReplyOnAcceptingConnections Only request a reply if the remote system is accepting connections
		void Ping( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections ) override;

		/// Returns the average of all ping times read for the specific system or -1 if none read yet
		/// \param[in] playerId Which system we are referring to
		/// \return The ping time for this system, or -1
		int GetAveragePing( const PlayerID playerId ) override;

		/// Returns the last ping time read for the specific system or -1 if none read yet
		/// \param[in] playerId Which system we are referring to
		/// \return The last ping time for this system, or -1
		int GetLastPing( const PlayerID playerId ) const override;

		/// Returns the lowest ping time read or -1 if none read yet
		/// \param[in] playerId Which system we are referring to
		/// \return The lowest ping time for this system, or -1
		int GetLowestPing( const PlayerID playerId ) const override;

		/// Ping the remote systems every so often, or not. This is off by default.  Can be called anytime.
		/// \param[in] doPing True to start occasional pings.  False to stop them.
		void SetOccasionalPing( bool doPing ) override;
		
		// --------------------------------------------------------------------------------------------Static Data Functions - Functions dealing with API defined synchronized memory--------------------------------------------------------------------------------------------
		/// All systems have a block of data associated with them, for user use.  This block of data can be used to easily 
		/// specify typical system data that you want to know on connection, such as the player's name.
		/// \param[in] playerId Which system you are referring to.  Pass the value returned by GetInternalID to refer to yourself
		/// \return The data passed to SetRemoteStaticData stored as a bitstream
		RakNet::BitStream * GetRemoteStaticData( const PlayerID playerId ) override;

		/// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
		/// specify typical system data that you want to know on connection, such as the player's name.
		/// \param[in] playerId Whose static data to change.  Use your own playerId to change your own static data
		/// \param[in] data a block of data to store
		/// \param[in] length The length of data in bytes 
		void SetRemoteStaticData( const PlayerID playerId, const char *data, const int length ) override;

		/// Sends your static data to the specified system. This is automatically done on connection.
		/// You should call this when you change your static data.To send the static data of another system (such as relaying their data) you should do this
		/// normally with Send)
		/// \param[in] target Who to send your static data to.  Specify UNASSIGNED_PLAYER_ID to broadcast to all
		void SendStaticData( const PlayerID target ) override;

		/// Sets the data to send along with a LAN server discovery or offline ping reply.
		/// \a length should be under 400 bytes, as a security measure against flood attacks
		/// \param[in] data a block of data to store, or 0 for none
		/// \param[in] length The length of data in bytes, or 0 for none
		/// \sa Ping.cpp
		void SetOfflinePingResponse( const char *data, const unsigned int length ) override;
		
		//--------------------------------------------------------------------------------------------Network Functions - Functions dealing with the network in general--------------------------------------------------------------------------------------------
		/// Return the unique address identifier that represents you on the the network and is based on your local IP / port.
		/// \return the identifier of your system internally, which may not be how other systems see if you if you are behind a NAT or proxy
		PlayerID GetInternalID( void ) const override;

		/// Return the unique address identifier that represents you on the the network and is based on your externalIP / port
		/// (the IP / port the specified player uses to communicate with you)
		/// \param[in] target Which remote system you are referring to for your external ID.  Usually the same for all systems, unless you have two or more network cards.
		PlayerID GetExternalID( const PlayerID target ) const override;

		/// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable message.
		/// Default time is 10,000 or 10 seconds in release and 30,000 or 30 seconds in debug.
		/// \param[in] timeMS Time, in MS
		/// \param[in] target Which system to do this for
		void SetTimeoutTime( RakNetTime timeMS, const PlayerID target ) override;

		/// Set the MTU per datagram.  It's important to set this correctly - otherwise packets will be needlessly split, decreasing performance and throughput.
		/// Maximum allowed size is MAXIMUM_MTU_SIZE.
		/// Too high of a value will cause packets not to arrive at worst and be fragmented at best.
		/// Too low of a value will split packets unnecessarily.
		/// Recommended size is 1500
		/// sa MTUSize.h
		/// \pre Can only be called when not connected.
		/// \return false on failure (we are connected), else true
		bool SetMTUSize( int size ) override;

		/// Returns the current MTU size
		/// \return The current MTU size
		int GetMTUSize( void ) const override;

		/// Returns the number of IP addresses this system has internally. Get the actual addresses from GetLocalIP()
		unsigned GetNumberOfAddresses( void ) override;

		/// Returns an IP address at index 0 to GetNumberOfAddresses-1
		const char* GetLocalIP( unsigned int index ) override;

		/// TODO - depreciate this
		/// Returns the dotted IP address for the specified playerId
		/// \param[in] playerId Any player ID other than UNASSIGNED_PLAYER_ID, even if that player is not currently connected
		const char* PlayerIDToDottedIP( const PlayerID playerId ) const override;

		// TODO - depreciate this
		/// Converts a dotted IP to a playerId
		/// \param[in]  host Either a dotted IP address or a domain name
		/// \param[in]  remotePort Which port to connect to on the remote machine.
		/// \param[out] playerId  The result of this operation
		void IPToPlayerID( const char* host, unsigned short remotePort, PlayerID *playerId ) override;

		/// Allow or disallow connection responses from any IP. Normally this should be false, but may be necessary
		/// when connecting to servers with multiple IP addresses.
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

		/// Controls how often to return ID_DOWNLOAD_PROGRESS for large message downloads.
		/// ID_DOWNLOAD_PROGRESS is returned to indicate a new partial message chunk, roughly the MTU size, has arrived
		/// As it can be slow or cumbersome to get this notification for every chunk, you can set the interval at which it is returned.
		/// Defaults to 0 (never return this notification)
		/// \param[in] interval How many messages to use as an interval
		void SetSplitMessageProgressInterval(int interval) override;

		/// Set how long to wait before giving up on sending an unreliable message
		/// Useful if the network is clogged up.
		/// Set to 0 or less to never timeout.  Defaults to 0.
		/// \param[in] timeoutMS How many ms to wait before simply not sending an unreliable message.
		void SetUnreliableTimeout(RakNetTime timeoutMS) override;

		// --------------------------------------------------------------------------------------------Compression Functions - Functions related to the compression layer--------------------------------------------------------------------------------------------
		/// Enables or disables frequency table tracking.  This is required to get a frequency table, which is used in GenerateCompressionLayer()
		/// This value persists between connect calls and defaults to false (no frequency tracking)
		/// \pre You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only trackpart of the values sent over the network.
		/// \param[in] doCompile True to enable tracking 
		void SetCompileFrequencyTable( bool doCompile ) override;

		/// Returns the frequency of outgoing bytes into output frequency table
		/// The purpose is to save to file as either a master frequency table from a sample game session for passing to
		/// GenerateCompressionLayer() 
		/// \pre You should only call this when disconnected. Requires that you first enable data frequency tracking by calling SetCompileFrequencyTable(true)
		/// \param[out] outputFrequencyTable  The frequency of each corresponding byte
		/// \return False (failure) if connected or if frequency table tracking is not enabled. Otherwise true (success)
		bool GetOutgoingFrequencyTable( unsigned int outputFrequencyTable[ 256 ] ) override;

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

		/// Returns the compression ratio. A low compression ratio is good.  Compression is for outgoing data
		/// \return The compression ratio
		float GetCompressionRatio( void ) const override;

		///Returns the decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
		/// \return The decompression ratio
		float GetDecompressionRatio( void ) const override;

		// -------------------------------------------------------------------------------------------- Plugin Functions--------------------------------------------------------------------------------------------
		/// Attatches a Plugin interface to run code automatically on message receipt in the Receive call
		/// \note If plugins have dependencies on each other then the order does matter - for example the router plugin should go first because it might route messages for other plugins
		/// \param[in] messageHandler Pointer to a plugin to attach
		void AttachPlugin( PluginInterface *plugin ) override;

		/// Detaches a Plugin interface to run code automatically on message receipt
		/// \param[in] messageHandler Pointer to a plugin to detach
		void DetachPlugin( PluginInterface *messageHandler ) override;

		// --------------------------------------------------------------------------------------------Miscellaneous Functions--------------------------------------------------------------------------------------------
		/// Put a message back at the end of the receive queue in case you don't want to deal with it immediately
		/// \param[in] packet The packet you want to push back.
		/// \param[in] pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
		void PushBackPacket( Packet *packet, bool pushAtHead ) override;

		/// \Internal
		// \param[in] routerInterface The router to use to route messages to systems not directly connected to this system.
		void SetRouterInterface( RouterInterface *routerInterface ) override;

		/// \Internal
		// \param[in] routerInterface The router to use to route messages to systems not directly connected to this system.
		void RemoveRouterInterface( RouterInterface *routerInterface ) override;

		// --------------------------------------------------------------------------------------------Network Simulator Functions--------------------------------------------------------------------------------------------
		/// Adds simulated ping and packet loss to the outgoing data flow.
		/// To simulate bi-directional ping and packet loss, you should call this on both the sender and the recipient, with half the total ping and maxSendBPS value on each.
		/// You can exclude network simulator code with the _RELEASE #define to decrease code size
		/// \param[in] maxSendBPS Maximum bits per second to send.  Packetloss grows linearly.  0 to disable.
		/// \param[in] minExtraPing The minimum time to delay sends.
		/// \param[in] extraPingVariance The additional random time to delay sends.
		void ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance) override;

		/// Returns if you previously called ApplyNetworkSimulator
		/// \return If you previously called ApplyNetworkSimulator
		bool IsNetworkSimulatorActive( void ) override;

		// --------------------------------------------------------------------------------------------Statistical Functions - Functions dealing with API performance--------------------------------------------------------------------------------------------

		/// Returns a structure containing a large set of network statistics for the specified system.
		/// You can map this data to a string using the C style StatisticsToString() function
		/// \param[in] playerId: Which connected system to get statistics for
		/// \return 0 on can't find the specified system.  A pointer to a set of data otherwise.
		/// \sa RakNetStatistics.h
		RakNetStatisticsStruct * GetStatistics( const PlayerID playerId ) override;

		// --------------------------------------------------------------------------------------------EVERYTHING AFTER THIS COMMENT IS FOR INTERNAL USE ONLY--------------------------------------------------------------------------------------------
		/// \internal
		RPCMap *GetRPCMap( const PlayerID playerId) override;

		/// \internal
		/// \brief Holds the clock differences between systems, along with the ping
		struct PingAndClockDifferential
		{
			unsigned short pingTime;
			RakNetTime clockDifferential;
		};

		/// \internal
		/// \brief All the information representing a connected system system
		struct RemoteSystemStruct
		{
			bool isActive; // Is this structure in use?
			PlayerID playerId;  /// The remote system associated with this reliability layer
			PlayerID myExternalPlayerId;  /// Your own IP, as reported by the remote system
			ReliabilityLayer reliabilityLayer;  /// The reliability layer associated with this player
			bool weInitiatedTheConnection; /// True if we started this connection via Connect.  False if someone else connected to us.
			PingAndClockDifferential pingAndClockDifferential[ PING_TIMES_ARRAY_SIZE ];  /// last x ping times and calculated clock differentials with it
			int pingAndClockDifferentialWriteIndex;  /// The index we are writing into the pingAndClockDifferential circular buffer
			unsigned short lowestPing; ///The lowest ping value encountered
			RakNetTime nextPingTime;  /// When to next ping this player
			RakNetTime lastReliableSend; /// When did the last reliable send occur.  Reliable sends must occur at least once every timeoutTime/2 units to notice disconnects
			RakNet::BitStream staticData; /// static data.  This cannot be a pointer because it might be accessed in another thread.
			RakNetTime connectionTime; /// connection time, if active.
			unsigned char AESKey[ 16 ]; /// Security key.
			bool setAESKey; /// true if security is enabled.
			RPCMap rpcMap; /// Mapping of RPC calls to single byte integers to save transmission bandwidth.
			enum ConnectMode {NO_ACTION, DISCONNECT_ASAP, DISCONNECT_ASAP_SILENTLY, DISCONNECT_ON_NO_ACK, REQUESTED_CONNECTION, HANDLING_CONNECTION_REQUEST, UNVERIFIED_SENDER, SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET, CONNECTED} connectMode;
		};

	protected:

	#ifdef _WIN32
		// friend unsigned __stdcall RecvFromNetworkLoop(LPVOID arguments);
		friend void __stdcall ProcessPortUnreachable( const unsigned int binaryAddress, const unsigned short port, RakPeer *rakPeer );
		friend void __stdcall ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer );
		friend unsigned __stdcall UpdateNetworkLoop( LPVOID arguments );
	#else
		// friend void*  RecvFromNetworkLoop( void*  arguments );
		friend void ProcessPortUnreachable( const unsigned int binaryAddress, const unsigned short port, RakPeer *rakPeer );
		friend void ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer );
		friend void* UpdateNetworkLoop( void* arguments );
	#endif

		// This is done to provide custom RPC handling when in a blocking RPC
		Packet* ReceiveIgnoreRPC( void );

		int GetIndexFromPlayerID( const PlayerID playerId, bool calledFromNetworkThread );

		//void RemoveFromRequestedConnectionsList( const PlayerID playerId );
		bool SendConnectionRequest( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength );
		///Get the reliability layer associated with a playerID.  
		/// \param[in] playerID The player identifier 
		/// \return 0 if none
		RemoteSystemStruct *GetRemoteSystemFromPlayerID( const PlayerID playerID, bool calledFromNetworkThread, bool onlyActive) const;
		///Parse out a connection request packet
		void ParseConnectionRequestPacket( RakPeer::RemoteSystemStruct *remoteSystem, PlayerID playerId, const char *data, int byteSize);
		///When we get a connection request from an ip / port, accept it unless full
		void OnConnectionRequest( RakPeer::RemoteSystemStruct *remoteSystem, unsigned char *AESKey, bool setAESKey );
		///Send a reliable disconnect packet to this player and disconnect them when it is delivered
		void NotifyAndFlagForDisconnect( const PlayerID playerId, bool performImmediate, unsigned char orderingChannel );
		///Returns how many remote systems initiated a connection to us
		unsigned short GetNumberOfRemoteInitiatedConnections( void ) const;
		///Get a free remote system from the list and assign our playerID to it.  Should only be called from the update thread - not the user thread
		RemoteSystemStruct * AssignPlayerIDToRemoteSystemList( const PlayerID playerId, RemoteSystemStruct::ConnectMode connectionMode );
		///An incoming packet has a timestamp, so adjust it to be relative to this system
		void ShiftIncomingTimestamp( unsigned char *data, PlayerID playerId ) const;
		///Get the most probably accurate clock differential for a certain player
		RakNetTime GetBestClockDifferential( const PlayerID playerId ) const;

		//void PushPortRefused( const PlayerID target );
		///Handles an RPC packet.  This is sending an RPC request
		/// \param[in] data A packet returned from Receive with the ID ID_RPC
		/// \param[in] length The size of the packet data 
		/// \param[in] playerId The sender of the packet 
		/// \return true on success, false on a bad packet or an unregistered function
		bool HandleRPCPacket( const char *data, int length, PlayerID playerId );

		///Handles an RPC reply packet.  This is data returned from an RPC call
		/// \param[in] data A packet returned from Receive with the ID ID_RPC
		/// \param[in] length The size of the packet data 
		/// \param[in] playerId The sender of the packet 
		void HandleRPCReplyPacket( const char *data, int length, PlayerID playerId );

	#ifdef __USE_IO_COMPLETION_PORTS

		bool SetupIOCompletionPortSocket( int index );
	#endif
		///Set this to true to terminate the Peer thread execution 
		volatile bool endThreads;
		///true if the peer thread is active. 
		volatile bool isMainLoopThreadActive;
		bool occasionalPing;  /// Do we occasionally ping the other systems?*/
		///Store the maximum number of peers allowed to connect
		unsigned short maximumNumberOfPeers;
		//05/02/06 Just using maximumNumberOfPeers instead
		///Store the maximum number of peers able to connect, including reserved connection slots for pings, etc.
		//unsigned short remoteSystemListSize;
		///Store the maximum incoming connection allowed 
		unsigned short maximumIncomingConnections;
		///localStaticData necessary because we may not have a RemoteSystemStruct representing ourselves in the list
		RakNet::BitStream localStaticData, offlinePingResponse;
		///Local PlayerImpl ID
		PlayerID myPlayerId;
		char incomingPassword[256];
		unsigned char incomingPasswordLength;

		/// This is an array of pointers to RemoteSystemStruct
		/// This allows us to preallocate the list when starting, so we don't have to allocate or delete at runtime.
		/// Another benefit is that is lets us add and remove active players simply by setting playerId
		/// and moving elements in the list by copying pointers variables without affecting running threads, even if they are in the reliability layer
		RemoteSystemStruct* remoteSystemList;

		// remoteSystemLookup is only used in the network thread or when single threaded.  Not safe to use this in the user thread
		// A list of PlayerIDAndIndex sorted by playerId.  This way, given a PlayerID, we can quickly get the index into remoteSystemList to find this player.
		// This is an optimization to avoid scanning the remoteSystemList::playerId member when doing targeted sends
		// Improves speed of a targeted send to every player from O(n^2) to O(n * log2(n)).
		// For an MMOG with 1000 players:
		// The original method takes 1000^2=1,000,000 calls
		// The new method takes:
		// logbx = (logax) / (logab)
		// log2(x) = log10(x) / log10(2)
		// log2(1000) = log10(1000) / log10(2)
		// log2(1000) = 9.965
		// 9.965 * 1000 = 9965
		// For 1000 players this optimization improves the speed by over 1000 times.
		DataStructures::OrderedList<PlayerID, PlayerIDAndIndex, PlayerIDAndIndexComp> remoteSystemLookup;
		
		enum
		{
			// Only put these mutexes in user thread functions!
	#ifdef _RAKNET_THREADSAFE
			transferToPacketQueue_Mutex,
			packetPool_Mutex,
			bufferedCommands_Mutex,
			requestedConnectionList_Mutex,
	#endif
			offlinePingResponse_Mutex,
			NUMBER_OF_RAKPEER_MUTEXES
		};
		SimpleMutex rakPeerMutexes[ NUMBER_OF_RAKPEER_MUTEXES ];
		///RunUpdateCycle is not thread safe but we don't need to mutex calls. Just skip calls if it is running already

		bool updateCycleIsRunning;
		///The list of people we have tried to connect to recently

		//DataStructures::Queue<RequestedConnectionStruct*> requestedConnectionsList;
		///Data that both the client and the server needs

		unsigned int bytesSentPerSecond, bytesReceivedPerSecond;
		// bool isSocketLayerBlocking;
		// bool continualPing,isRecvfromThreadActive,isMainLoopThreadActive, endThreads, isSocketLayerBlocking;
		unsigned int validationInteger;
	#ifdef _WIN32
		HANDLE
	#else
		pthread_t
	#endif
			processPacketsThreadHandle, recvfromThreadHandle;
		SimpleMutex incomingQueueMutex, banListMutex; //,synchronizedMemoryQueueMutex, automaticVariableSynchronizationMutex;
		//DataStructures::Queue<Packet *> incomingpacketSingleProducerConsumer; //, synchronizedMemorypacketSingleProducerConsumer;
		// BitStream enumerationData;

		/// @brief Automatic Variable Synchronization Mechanism
		/// automatic variable synchronization takes a primary and secondary identifier
		/// The unique primary identifier is the index into the automaticVariableSynchronizationList
		/// The unique secondary identifier (UNASSIGNED_NETWORK_ID for none) is in an unsorted list of memory blocks
		struct MemoryBlock
		{
			char *original, *copy;
			unsigned short size;
			NetworkID secondaryID;
			bool isAuthority;
			bool ( *synchronizationRules ) ( char*, char* );
		};

		struct BanStruct
		{
			char *IP;
			RakNetTime timeout; // 0 for none
		};

		struct RequestedConnectionStruct
		{
			PlayerID playerId;
			RakNetTime nextRequestTime;
			unsigned char requestsMade;
			char *data;
			unsigned short dataLength;
			char outgoingPassword[256];
			unsigned char outgoingPasswordLength;
			enum {CONNECT=1, /*PING=2, PING_OPEN_CONNECTIONS=4,*/ /*ADVERTISE_SYSTEM=2*/} actionToTake;
		};

		//DataStructures::List<DataStructures::List<MemoryBlock>* > automaticVariableSynchronizationList;
		DataStructures::List<BanStruct*> banList;
		DataStructures::List<PluginInterface*> messageHandlerList;
		DataStructures::SingleProducerConsumer<RequestedConnectionStruct> requestedConnectionList;

		/// Compression stuff
		unsigned int frequencyTable[ 256 ];
		DataStructures::HuffmanEncodingTree *inputTree, *outputTree;
		unsigned int rawBytesSent, rawBytesReceived, compressedBytesSent, compressedBytesReceived;
		// void DecompressInput(RakNet::BitStream *bitStream);
		// void UpdateOutgoingFrequencyTable(RakNet::BitStream * bitStream);
		void GenerateSYNCookieRandomNumber( void );
		void SecuredConnectionResponse( const PlayerID playerId );
		void SecuredConnectionConfirmation( RakPeer::RemoteSystemStruct * remoteSystem, char* data );
		bool RunUpdateCycle( void );
		// void RunMutexedUpdateCycle(void);

		struct BufferedCommandStruct
		{
			int numberOfBitsToSend;
			PacketPriority priority;
			PacketReliability reliability;
			char orderingChannel;
			PlayerID playerId;
			bool broadcast;
			RemoteSystemStruct::ConnectMode connectionMode;
			NetworkID networkID;
			bool blockingCommand; // Only used for RPC
			char *data;
			enum {BCS_SEND, BCS_CLOSE_CONNECTION, /*BCS_RPC, BCS_RPC_SHIFT,*/ BCS_DO_NOTHING} command;
		};

		// Single producer single consumer queue using a linked list
		//BufferedCommandStruct* bufferedCommandReadIndex, bufferedCommandWriteIndex;
		DataStructures::SingleProducerConsumer<BufferedCommandStruct> bufferedCommands;

		bool AllowIncomingConnections(void) const;

		// Sends static data using immediate send mode or not (if called from user thread, put false for performImmediate.  If called from update thread, put true).
		// This is done for efficiency, so we don't buffer send calls that are from the network thread anyway
		void SendStaticDataInternal( const PlayerID target, bool performImmediate );
		void PingInternal( const PlayerID target, bool performImmediate );
		bool ValidSendTarget(PlayerID playerId, bool broadcast);
		// This stores the user send calls to be handled by the update thread.  This way we don't have thread contention over playerIDs
		void CloseConnectionInternal( const PlayerID target, bool sendDisconnectionNotification, bool performImmediate, unsigned char orderingChannel );
		void SendBuffered( const char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, RemoteSystemStruct::ConnectMode connectionMode );
		bool SendImmediate( char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool useCallerDataAllocation, RakNetTimeNS currentTime );
		//bool HandleBufferedRPC(BufferedCommandStruct *bcs, RakNetTime time);
		void ClearBufferedCommands(void);
		void ClearRequestedConnectionList(void);
		void AddPacketToProducer(Packet *p);

		//DataStructures::AVLBalancedBinarySearchTree<RPCNode> rpcTree;
		RPCMap rpcMap; // Can't use StrPtrHash because runtime insertions will screw up the indices
		int MTUSize;
		bool trackFrequencyTable;
		int threadSleepTimer;

		SOCKET connectionSocket;
	#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
		WSAEVENT recvEvent;
	#endif

		// Used for RPC replies
		RakNet::BitStream *replyFromTargetBS;
		PlayerID replyFromTargetPlayer;
		bool replyFromTargetBroadcast;

		// Problem:
		// Waiting in function A:
		// Wait function gets RPC B:
		// 
		bool blockOnRPCReply;

		// For redirecting sends through the router plugin.  Unfortunate I have to use this architecture.
		RouterInterface *router;

		// Nobody would use the internet simulator in a final build.
	#ifndef _RELEASE
		double _maxSendBPS;
		unsigned short _minExtraPing, _extraPingVariance;
	#endif

	#if !defined(_COMPATIBILITY_1)
		/// Encryption and security
		big::RSACrypt<RSA_BIT_SIZE> rsacrypt;
		big::u32 publicKeyE;
		RSA_BIT_SIZE publicKeyN;
		bool keysLocallyGenerated, usingSecurity;
		RakNetTime randomNumberExpirationTime;
		unsigned char newRandomNumber[ 20 ], oldRandomNumber[ 20 ];
	#endif
		
		///How long it has been since things were updated by a call to receiveUpdate thread uses this to determine how long to sleep for
		//unsigned int lastUserUpdateCycle;
		/// True to allow connection accepted packets from anyone.  False to only allow these packets from servers we requested a connection to.
		bool allowConnectionResponseIPMigration;

		int splitMessageProgressInterval;
		RakNetTime unreliableTimeout;

		// The packetSingleProducerConsumer transfers the packets from the network thread to the user thread. The pushedBackPacket holds packets that couldn't be processed
		// immediately while waiting on blocked RPCs
		DataStructures::SingleProducerConsumer<Packet*> packetSingleProducerConsumer;
		//DataStructures::Queue<Packet*> pushedBackPacket, outOfOrderDeallocatedPacket;
		DataStructures::Queue<Packet*> packetPool;
	};
}

#endif
