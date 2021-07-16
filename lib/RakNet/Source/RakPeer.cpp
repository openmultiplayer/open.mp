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

#include "RakNetDefines.h"
#include "RakPeer.h"
#include "NetworkTypes.h"

#include "../SAMPRakNet.hpp"

#ifdef __USE_IO_COMPLETION_PORTS
#include "AsynchronousFileIO.h"
#endif

#ifdef _WIN32
//#include <Shlwapi.h>
#include <process.h>
#else
#define closesocket close
#include <unistd.h>
#include <pthread.h>
#endif
#include <ctype.h> // toupper
#include <string.h>
#include "GetTime.h"
#include "PacketEnumerations.h"
#include "DS_HuffmanEncodingTree.h"
#include "Rand.h"
#include "PluginInterface.h"
#include "StringCompressor.h"
#include "StringTable.h"
#include "NetworkIDGenerator.h"
#include "NetworkTypes.h"
#include "SHA1.h"
#include "RakSleep.h"
#include "RouterInterface.h"
#include "RakAssert.h"

#if !defined ( __APPLE__ ) && !defined ( __APPLE_CC__ )
#include <malloc.h>
#endif

#ifdef _COMPATIBILITY_1
//
#elif defined(_WIN32)
//
#elif defined(_COMPATIBILITY_2)
#include "Compatibility2Includes.h"
#include <stdlib.h>
#else
#include <stdlib.h>
#endif

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning( push )
#endif

namespace RakNet
{
	#ifdef _WIN32
	unsigned __stdcall UpdateNetworkLoop(LPVOID arguments);
	#else
	void* UpdateNetworkLoop(void* arguments);
	#endif

	int PlayerIDAndIndexComp(const PlayerID &key, const PlayerIDAndIndex &data)
	{
		if (key < data.playerId)
			return -1;
		if (key == data.playerId)
			return 0;
		return 1;
	}

	// On a Little-endian machine the RSA key and message are mangled, but we're
	// trying to be friendly to the little endians, so we do byte order
	// mangling on Big-Endian machines.  Note that this mangling is independent
	// of the byte order used on the network (which also defaults to little-end).
	#ifdef HOST_ENDIAN_IS_BIG
	void __inline BSWAPCPY(unsigned char *dest, unsigned char *source, int bytesize)
	{

		RakAssert((bytesize % 4 == 0) && (bytesize) && "Something is wrong with your exponent or modulus size.");

		int i;
		for (i = 0; i < bytesize; i += 4)
		{
			dest[i] = source[i + 3];
			dest[i + 1] = source[i + 2];
			dest[i + 2] = source[i + 1];
			dest[i + 3] = source[i];
		}
	}
	void __inline BSWAPSELF(unsigned char *source, int bytesize)
	{

		RakAssert((bytesize % 4 == 0) && (bytesize) && "Something is wrong with your exponent or modulus size.");

		int i;
		unsigned char a, b;
		for (i = 0; i < bytesize; i += 4)
		{
			a = source[i];
			b = source[i + 1];
			source[i] = source[i + 3];
			source[i + 1] = source[i + 2];
			source[i + 2] = b;
			source[i + 3] = a;
		}
	}
	#endif
}

static const unsigned int SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION = 5000;
static const int MAX_OFFLINE_DATA_LENGTH=400; // I set this because I limit ID_CONNECTION_REQUEST to 512 bytes, and the password is appended to that packet.

//#define _DO_PRINTF

// UPDATE_THREAD_POLL_TIME is how often the update thread will poll to see
// if receive wasn't called within UPDATE_THREAD_UPDATE_TIME.  If it wasn't called within that time,
// the updating thread will activate and take over network communication until Receive is called again.
//static const unsigned int UPDATE_THREAD_UPDATE_TIME=30;
//static const unsigned int UPDATE_THREAD_POLL_TIME=30;

//#define _TEST_AES

namespace RakNet
{
	Packet *AllocPacket(unsigned dataSize)
	{
		Packet *p = (Packet *)malloc(sizeof(Packet) + dataSize);
		p->data = (unsigned char*)p + sizeof(Packet);
		p->length = dataSize;
		p->deleteData = false;
		return p;
	}

	Packet *AllocPacket(unsigned dataSize, unsigned char *data)
	{
		Packet *p = (Packet *)malloc(sizeof(Packet));
		p->data = data;
		p->length = dataSize;
		p->deleteData = true;
		return p;
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RakPeer()
{
	StringCompressor::AddReference();
	StringTable::AddReference();

#if !defined(_COMPATIBILITY_1)
	usingSecurity = false;
#endif
	memset( frequencyTable, 0, sizeof( unsigned int ) * 256 );
	rawBytesSent = rawBytesReceived = compressedBytesSent = compressedBytesReceived = 0;
	outputTree = inputTree = 0;
	connectionSocket = INVALID_SOCKET;
	MTUSize = DEFAULT_MTU_SIZE;
	trackFrequencyTable = false;
	maximumIncomingConnections = 0;
	maximumNumberOfPeers = 0;
	//remoteSystemListSize=0;
	remoteSystemList = 0;
	bytesSentPerSecond = bytesReceivedPerSecond = 0;
	endThreads = true;
	isMainLoopThreadActive = false;
	// isRecvfromThreadActive=false;
	occasionalPing = false;
	connectionSocket = INVALID_SOCKET;
	myPlayerId = UNASSIGNED_PLAYER_ID;
	allowConnectionResponseIPMigration = false;
	blockOnRPCReply=false;
	//incomingPasswordLength=outgoingPasswordLength=0;
	incomingPasswordLength=0;
	router=0;
	splitMessageProgressInterval=0;
	unreliableTimeout=0;

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	recvEvent = INVALID_HANDLE_VALUE;
#endif

#ifndef _RELEASE
	_maxSendBPS=0.0;
	_minExtraPing=0;
	_extraPingVariance=0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::~RakPeer()
{
//	unsigned i;

	// Free the ban list.
	ClearBanList();

	Disconnect( 0, 0);


	StringCompressor::RemoveReference();
	StringTable::RemoveReference();
	SocketLayer::Destroy();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Starts the network threads, opens the listen port
// You must call this before calling SetMaximumIncomingConnections or Connect
// Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Disconnect()
//
// Parameters:
// maxConnections:  Required so the network can preallocate and for thread safety.
// - A pure client would set this to 1.  A pure server would set it to the number of allowed clients.
// - A hybrid would set it to the sum of both types of connections
// localPort: The port to listen for connections on.
// _threadSleepTimer: How many ms to Sleep each internal update cycle (30 to give the game priority, 0 for regular (recommended), -1 to not Sleep() (may be slower))
 // forceHostAddress Can force RakNet to use a particular IP to host on.  Pass 0 to automatically pick an IP
// Returns:
// False on failure (can't create socket or thread), true on success.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Initialize( unsigned short maxConnections, unsigned short localPort, int _threadSleepTimer, const char *forceHostAddress )
{
	if (IsActive())
		return false;

	unsigned i;

	RakAssert( maxConnections > 0 );

	if ( maxConnections <= 0 )
		return false;

	if ( connectionSocket == INVALID_SOCKET )
	{
		connectionSocket = SocketLayer::Instance()->CreateBoundSocket( localPort, true, forceHostAddress );

		if ( connectionSocket == INVALID_SOCKET )
			return false;

		unsigned short localPort2 = SocketLayer::Instance()->GetLocalPort(connectionSocket);
		if (localPort2!=0)
			localPort=localPort2;
	}

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	if (_threadSleepTimer>0)
	{
		recvEvent=CreateEvent(0,FALSE,FALSE,0);
		WSAEventSelect(connectionSocket,recvEvent,FD_READ);
	}	
#endif

	if ( maximumNumberOfPeers == 0 )
	{
		// Don't allow more incoming connections than we have peers.
		if ( maximumIncomingConnections > maxConnections )
			maximumIncomingConnections = maxConnections;

		maximumNumberOfPeers = maxConnections;
		// 04/19/2006 - Don't overallocate because I'm not longer allowing connected pings.
		// The disconnects are not consistently processed and the process was sloppy and complicated.
		// Allocate 10% extra to handle new connections from players trying to connect when the server is full
		//remoteSystemListSize = maxConnections;// * 11 / 10 + 1;

		// remoteSystemList in Single thread
		//remoteSystemList = new RemoteSystemStruct[ remoteSystemListSize ];
		remoteSystemList = new RemoteSystemStruct[ maximumNumberOfPeers ];


		for ( i = 0; i < maximumNumberOfPeers; i++ )
		//for ( i = 0; i < remoteSystemListSize; i++ )
		{
			// remoteSystemList in Single thread
			remoteSystemList[ i ].isActive = false;
			#ifndef _RELEASE
			remoteSystemList[ i ].reliabilityLayer.ApplyNetworkSimulator(_maxSendBPS, _minExtraPing, _extraPingVariance);
			#endif
		}

		// Clear the lookup table.  Safe to call from the user thread since the network thread is now stopped
		remoteSystemLookup.Clear();
	}

	// For histogram statistics
	// nextReadBytesTime=0;
	// lastSentBytes=lastReceivedBytes=0;

	if ( endThreads )
	{
	//	lastUserUpdateCycle = 0;

		// Reset the frequency table that we use to save outgoing data
		memset( frequencyTable, 0, sizeof( unsigned int ) * 256 );

		// Reset the statistical data
		rawBytesSent = rawBytesReceived = compressedBytesSent = compressedBytesReceived = 0;

		updateCycleIsRunning = false;
		endThreads = false;
		// Create the threads
		threadSleepTimer = _threadSleepTimer;

		ClearBufferedCommands();

#if !defined(_COMPATIBILITY_1)
		char ipList[ 10 ][ 16 ];
		SocketLayer::Instance()->GetMyIP( ipList );
		myPlayerId.port = localPort;
		if (forceHostAddress==0 || forceHostAddress[0]==0)
			myPlayerId.binaryAddress = inet_addr( ipList[ 0 ] );
		else
			myPlayerId.binaryAddress = inet_addr( forceHostAddress );
#else
		myPlayerId=UNASSIGNED_PLAYER_ID;
#endif
		{
#ifdef _WIN32

			if ( isMainLoopThreadActive == false )
			{
				unsigned ProcessPacketsThreadID = 0;
#ifdef _COMPATIBILITY_1
				processPacketsThreadHandle = ( HANDLE ) _beginthreadex( NULL, 0, UpdateNetworkLoop, this, 0, &ProcessPacketsThreadID );
#else
				processPacketsThreadHandle = ( HANDLE ) _beginthreadex( NULL, MAX_ALLOCA_STACK_ALLOCATION*2, UpdateNetworkLoop, this, 0, &ProcessPacketsThreadID );
#endif

				//BOOL b =  SetThreadPriority(
				//	processPacketsThreadHandle,
				//	THREAD_PRIORITY_HIGHEST
				//	);

				if ( processPacketsThreadHandle == 0 )
				{
					Disconnect( 0, 0 );
					return false;
				}

				// SetThreadPriority(processPacketsThreadHandle, THREAD_PRIORITY_HIGHEST);

				CloseHandle( processPacketsThreadHandle );

				processPacketsThreadHandle = 0;

			}

#else
			pthread_attr_t attr;
			pthread_attr_init( &attr );
			pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

			//  sched_param sp;
			//  sp.sched_priority = sched_get_priority_max(SCHED_OTHER);
			//  pthread_attr_setschedparam(&attr, &sp);

			int error;

			if ( isMainLoopThreadActive == false )
			{
				error = pthread_create( &processPacketsThreadHandle, &attr, &UpdateNetworkLoop, this );

				if ( error )
				{
					Disconnect( 0 );
					return false;
				}
			}

			processPacketsThreadHandle = 0;
#endif


			// Wait for the threads to activate.  When they are active they will set these variables to true

			while (  /*isRecvfromThreadActive==false || */isMainLoopThreadActive == false )
				RakSleep(10);

		}
	}

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->OnInitialize(this);
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Must be called while offline
// Secures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to prevent
// connection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.
// There is a significant amount of processing and a slight amount of bandwidth
// overhead for this feature.
//
// If you accept connections, you must call this or else secure connections will not be enabled
// for incoming connections.
// If you are connecting to another system, you can call this with values for the
// (e and p,q) public keys before connecting to prevent MitM
//
// Parameters:
// pubKeyE, pubKeyN - A pointer to the public keys from the RSACrypt class. See the Encryption sample
// privKeyP, privKeyQ - Private keys generated from the RSACrypt class.  See the Encryption sample
// If the private keys are 0, then a new key will be generated when this function is called
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::InitializeSecurity(const char *pubKeyE, const char *pubKeyN, const char *privKeyP, const char *privKeyQ )
{
#if !defined(_COMPATIBILITY_1)
	if ( endThreads == false )
		return ;

	// Setting the client key is e,n,
	// Setting the server key is p,q
	if ( //( privKeyP && privKeyQ && ( pubKeyE || pubKeyN ) ) ||
		//( pubKeyE && pubKeyN && ( privKeyP || privKeyQ ) ) ||
		( privKeyP && privKeyQ == 0 ) ||
		( privKeyQ && privKeyP == 0 ) ||
		( pubKeyE && pubKeyN == 0 ) ||
		( pubKeyN && pubKeyE == 0 ) )
	{
		// Invalid parameters
		RakAssert( 0 );
	}

	seedMT( (unsigned int) RakNet::GetTime() );

	GenerateSYNCookieRandomNumber();

	usingSecurity = true;

	if ( privKeyP == 0 && privKeyQ == 0 && pubKeyE == 0 && pubKeyN == 0 )
	{
		keysLocallyGenerated = true;
		rsacrypt.generateKeys();
	}

	else
	{
		if ( pubKeyE && pubKeyN )
		{
			// Save public keys
			memcpy( ( char* ) & publicKeyE, pubKeyE, sizeof( publicKeyE ) );
			memcpy( publicKeyN, pubKeyN, sizeof( publicKeyN ) );
		}

		if ( privKeyP && privKeyQ )
		{
			BIGHALFSIZE( RSA_BIT_SIZE, p );
			BIGHALFSIZE( RSA_BIT_SIZE, q );
			memcpy( p, privKeyP, sizeof( p ) );
			memcpy( q, privKeyQ, sizeof( q ) );
			// Save private keys
			rsacrypt.setPrivateKey( p, q );
		}

		keysLocallyGenerated = false;
	}
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description
// Must be called while offline
// Disables all security.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DisableSecurity( void )
{
#if !defined(_COMPATIBILITY_1)
	if ( endThreads == false )
		return ;

	usingSecurity = false;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets how many incoming connections are allowed.  If this is less than the number of players currently connected, no
// more players will be allowed to connect.  If this is greater than the maximum number of peers allowed, it will be reduced
// to the maximum number of peers allowed.  Defaults to 0.
//
// Parameters:
// numberAllowed - Maximum number of incoming connections allowed.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetMaximumIncomingConnections( unsigned short numberAllowed )
{
	maximumIncomingConnections = numberAllowed;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the maximum number of incoming connections, which is always <= maxConnections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumIncomingConnections( void ) const
{
	return maximumIncomingConnections;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets the password incoming connections must match in the call to Connect (defaults to none)
// Pass 0 to passwordData to specify no password
//
// Parameters:
// passwordData: A data block that incoming connections must match.  This can be just a password, or can be a stream of data.
// - Specify 0 for no password data
// passwordDataLength: The length in bytes of passwordData
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetIncomingPassword( const char* passwordData, int passwordDataLength )
{
	//if (passwordDataLength > MAX_OFFLINE_DATA_LENGTH)
	//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;

	if (passwordDataLength > 255)
		passwordDataLength=255;

	if (passwordData==0)
		passwordDataLength=0;

	// Not threadsafe but it's not important enough to lock.  Who is going to change the password a lot during runtime?
	// It won't overflow at least because incomingPasswordLength is an unsigned char
	if (passwordDataLength>0)
		memcpy(incomingPassword, passwordData, passwordDataLength);
	incomingPasswordLength=(unsigned char)passwordDataLength;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GetIncomingPassword( char* passwordData, int *passwordDataLength  )
{
	if (passwordData==0)
	{
		*passwordDataLength=incomingPasswordLength;
		return;
	}

	if (*passwordDataLength > incomingPasswordLength)
		*passwordDataLength=incomingPasswordLength;

	if (*passwordDataLength>0)
		memcpy(passwordData, incomingPassword, *passwordDataLength);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to connect to the specified host (ip or domain name) and server port.
// Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.  Calling both acts as a true peer.
// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
// or receive gets a packet with the type identifier ID_CONNECTION_ACCEPTED.  If the connection is not
// successful, such as rejected connection or no response then neither of these things will happen.
// Requires that you first call Initialize
//
// Parameters:
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// passwordData: A data block that must match the data block on the server.  This can be just a password, or can be a stream of data
// passwordDataLength: The length in bytes of passwordData
//
// Returns:
// True on successful initiation. False on incorrect parameters, internal error, or too many existing peers
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Connect( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	// If endThreads is true here you didn't call Initialize() first.
	if ( host == 0 || endThreads || connectionSocket == INVALID_SOCKET )
		return false;

	unsigned numberOfFreeSlots;

	numberOfFreeSlots = 0;

	//if (passwordDataLength>MAX_OFFLINE_DATA_LENGTH)
	//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;
	if (passwordDataLength>255)
		passwordDataLength=255;

	if (passwordData==0)
		passwordDataLength=0;

	// Not threadsafe but it's not important enough to lock.  Who is going to change the password a lot during runtime?
	// It won't overflow at least because outgoingPasswordLength is an unsigned char
//	if (passwordDataLength>0)
//		memcpy(outgoingPassword, passwordData, passwordDataLength);
//	outgoingPasswordLength=(unsigned char) passwordDataLength;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return false;
#endif
		if (host==0)
			return false;
	}

	// Connecting to ourselves in the same instance of the program?
	if ( ( strcmp( host, "127.0.0.1" ) == 0 || strcmp( host, "0.0.0.0" ) == 0 ) && remotePort == myPlayerId.port )
		return false;

	return SendConnectionRequest( host, remotePort, passwordData, passwordDataLength );
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Stops the network threads and close all connections.  Multiple calls are ok.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Disconnect( unsigned int blockDuration, unsigned char orderingChannel )
{
	unsigned i,j;
	bool anyActive;
	RakNetTime startWaitingTime;
//	PlayerID playerId;
	RakNetTime time;
	//unsigned short systemListSize = remoteSystemListSize; // This is done for threading reasons
	unsigned short systemListSize = maximumNumberOfPeers;

	if ( blockDuration > 0 )
	{
		for ( i = 0; i < systemListSize; i++ )
		{
			// remoteSystemList in user thread
			NotifyAndFlagForDisconnect(remoteSystemList[i].playerId, false, orderingChannel);
		}

		time = RakNet::GetTime();
		startWaitingTime = time;
		while ( time - startWaitingTime < blockDuration )
		{
			anyActive=false;
			for (j=0; j < systemListSize; j++)
			{
				// remoteSystemList in user thread
				if (remoteSystemList[j].isActive)
				{
					anyActive=true;
					break;
				}
			}

			// If this system is out of packets to send, then stop waiting
			if ( anyActive==false )
				break;

			// This will probably cause the update thread to run which will probably
			// send the disconnection notification

			RakSleep(15);
			time = RakNet::GetTime();
		}
	}

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->OnDisconnect(this);
	}

	if ( endThreads == false )
	{
		// Stop the threads
		endThreads = true;

		// Normally the thread will call DecreaseUserCount on termination but if we aren't using threads just do it
		// manually
#ifdef __USE_IO_COMPLETION_PORTS
		AsynchronousFileIO::Instance()->DecreaseUserCount();
#endif
	}

	while ( isMainLoopThreadActive )
		RakSleep(15);

	// remoteSystemList in Single thread
	for ( i = 0; i < systemListSize; i++ )
	{
		// Reserve this reliability layer for ourselves
		remoteSystemList[ i ].isActive = false;

		// Remove any remaining packets
		remoteSystemList[ i ].reliabilityLayer.Reset(false);
	}

	// Clear the lookup table.  Safe to call from the user thread since the network thread is now stopped
	remoteSystemLookup.Clear();

	// Setting maximumNumberOfPeers to 0 allows remoteSystemList to be reallocated in Initialize.
	// Setting remoteSystemListSize prevents threads from accessing the reliability layer
	maximumNumberOfPeers = 0;
	//remoteSystemListSize = 0;

	// Free any packets the user didn't deallocate
	Packet **packet;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
	packet=packetSingleProducerConsumer.ReadLock();
	while (packet)
	{
		DeallocatePacket(*packet);
		packetSingleProducerConsumer.ReadUnlock();
		packet=packetSingleProducerConsumer.ReadLock();
	}
	packetSingleProducerConsumer.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	for (i=0; i < packetPool.Size(); i++)
		DeallocatePacket(packetPool[i]);
	packetPool.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif

	blockOnRPCReply=false;

	if ( connectionSocket != INVALID_SOCKET )
	{
		closesocket( connectionSocket );
		connectionSocket = INVALID_SOCKET;
	}

	ClearBufferedCommands();
	bytesSentPerSecond = bytesReceivedPerSecond = 0;

	ClearRequestedConnectionList();

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	if (recvEvent!=INVALID_HANDLE_VALUE)
	{
		CloseHandle( recvEvent );
		recvEvent = INVALID_HANDLE_VALUE;
	}	
#endif

	// Clear out the reliability layer list in case we want to reallocate it in a successive call to Init.
	RemoteSystemStruct * temp = remoteSystemList;
	remoteSystemList = 0;
	delete [] temp;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns true if the network threads are running
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline bool RakPeer::IsActive( void ) const
{
	return endThreads == false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Fills the array remoteSystems with the playerID of all the systems we are connected to
//
// Parameters:
// remoteSystems (out): An array of PlayerID structures to be filled with the PlayerIDs of the systems we are connected to
// - pass 0 to remoteSystems to only get the number of systems we are connected to
// numberOfSystems (int, out): As input, the size of remoteSystems array.  As output, the number of elements put into the array
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetConnectionList( PlayerID *remoteSystems, unsigned short *numberOfSystems ) const
{
	int count, index;
	count=0;

	if ( remoteSystemList == 0 || endThreads == true )
	{
		*numberOfSystems = 0;
		return false;
	}

	// This is called a lot so I unrolled the loop
	if ( remoteSystems )
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
			{
				if ( count < *numberOfSystems )
					remoteSystems[ count ] = remoteSystemList[ index ].playerId;

				++count;
			}
	}
	else
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
				++count;
	}

	*numberOfSystems = ( unsigned short ) count;

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends a block of data to the specified system that you are connected to.
// This function only works while the client is connected (Use the Connect function).
//
// Parameters:
// data: The block of data to send
// length: The size in bytes of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingChannel: When using ordered or sequenced packets, what channel to order these on.
// - Packets are only ordered relative to other packets on the same stream
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to. Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// Returns:
// False if we are not connected to the specified recipient.  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{

	RakAssert( data && length > 0 );


	if ( data == 0 || length < 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send(data, BYTES_TO_BITS(length), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		SendBuffered(data, length*8, priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}

	return true;
}

bool RakPeer::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{

	RakAssert( bitStream->GetNumberOfBytesUsed() > 0 );


	if ( bitStream->GetNumberOfBytesUsed() == 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		// Sends need to be buffered and processed in the update thread because the playerID associated with the reliability layer can change,
		// from that thread, resulting in a send to the wrong player!  While I could mutex the playerID, that is much slower than doing this
		SendBuffered((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}
	
	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Gets a packet from the incoming packet queue. Use DeallocatePacket to deallocate the packet after you are done with it.  Packets must be deallocated in the same order they are received.
// Check the Packet struct at the top of CoreNetworkStructures.h for the format of the struct
//
// Returns:
// 0 if no packets are waiting to be handled, otherwise an allocated packet
// If the client is not active this will also return 0, as all waiting packets are flushed when the client is Disconnected
// This also updates all memory blocks associated with synchronized memory and distributed objects
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Packet* RakPeer::Receive( void )
{
	Packet *packet = ReceiveIgnoreRPC();
	while (packet && (packet->data[ 0 ] == ID_RPC || (packet->length>sizeof(unsigned char)+sizeof(RakNetTime) && packet->data[0]==ID_TIMESTAMP && packet->data[sizeof(unsigned char)+sizeof(RakNetTime)]==ID_RPC)))
	{
		// Do RPC calls from the user thread, not the network update thread
		// If we are currently blocking on an RPC reply, send ID_RPC to the blocker to handle rather than handling RPCs automatically
		HandleRPCPacket( ( char* ) packet->data, packet->length, packet->playerId );
		DeallocatePacket( packet );

		packet = ReceiveIgnoreRPC();
	}

    return packet;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal - Gets a packet without checking for RPCs
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
Packet* RakPeer::ReceiveIgnoreRPC( void )
{
	if ( !( IsActive() ) )
		return 0;

	Packet *packet;
	Packet **threadPacket;
	PluginReceiveResult pluginResult;

	int offset;
	unsigned int i;

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->Update(this);
	}

	do 
	{
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
		// Take all the messages off the queue so if the user pushes them back they are really pushed back, and not just at the end of the immediate write
		threadPacket=packetSingleProducerConsumer.ReadLock();
		while (threadPacket)
		{
			packet=*threadPacket;
			packetSingleProducerConsumer.ReadUnlock();
			threadPacket=packetSingleProducerConsumer.ReadLock();
			packetPool.Push(packet);
		}
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Lock();
#endif
		if (packetPool.Size()==0)
		{
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
			return 0;
		}

		packet = packetPool.Pop();
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
		if ( ( packet->length >= sizeof(unsigned char) + sizeof( RakNetTime ) ) &&
			( (unsigned char) packet->data[ 0 ] == ID_TIMESTAMP ) )
		{
			offset = sizeof(unsigned char);
			ShiftIncomingTimestamp( packet->data + offset, packet->playerId );
		}
		if ( (unsigned char) packet->data[ 0 ] == ID_RPC_REPLY )
		{
			HandleRPCReplyPacket( ( char* ) packet->data, packet->length, packet->playerId );
			DeallocatePacket( packet );
			packet=0; // Will do the loop again and get another packet
		}
		else
		{
			for (i=0; i < messageHandlerList.Size(); i++)
			{
				pluginResult=messageHandlerList[i]->OnReceive(this, packet);
				if (pluginResult==RR_STOP_PROCESSING_AND_DEALLOCATE)
				{
					DeallocatePacket( packet );
					packet=0; // Will do the loop again and get another packet
					break; // break out of the enclosing for
				}
				else if (pluginResult==RR_STOP_PROCESSING)
				{
					packet=0;
					break;
				}
			}
		}

	} while(packet==0);


	RakAssert( packet->data );


	return packet;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to deallocate a packet returned by Receive, in the same order returned to you from Receive
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DeallocatePacket( Packet *packet )
{
	if ( packet == 0 )
		return;

	if (packet->deleteData)
		delete packet->data;
	free(packet);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the total number of connections we are allowed
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumNumberOfPeers( void ) const
{
	return maximumNumberOfPeers;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Register a C function as available for calling as a remote procedure call
//
// Parameters:
// uniqueID: A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions
// functionName(...): The name of the C function or C++ singleton to be used as a function pointer
// This can be called whether the client is active or not, and registered functions stay registered unless unregistered with
// UnregisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RegisterAsRemoteProcedureCall( RPCID  uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms, void* extra ), void* extra)
{
	if ( uniqueID == 0 ||
#if RPCID_STRING
			uniqueID[ 0 ] == 0 ||
#endif
			functionPointer == 0 )
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID, (void*)functionPointer, false, extra);

	/*
	char uppercaseUniqueID[ 256 ];

	int counter = 0;

	while ( uniqueID[ counter ] )
	{
		uppercaseUniqueID[ counter ] = ( char ) toupper( uniqueID[ counter ] );
		counter++;
	}

	uppercaseUniqueID[ counter ] = 0;

	// Each id must be unique
//#ifdef _DEBUG
//	RakAssert( rpcTree.IsIn( RPCNode( uppercaseUniqueID, functionName ) ) == false );
//#endif

	if (rpcTree.IsIn( RPCNode( uppercaseUniqueID, functionName ) ))
		return;

	rpcTree.Add( RPCNode( uppercaseUniqueID, functionName ) );
	*/
}

void RakPeer::RegisterClassMemberRPC( RPCID  uniqueID, void *functionPointer )
{
	if ( uniqueID == 0 ||
#if RPCID_STRING
			uniqueID[ 0 ] == 0 ||
#endif
			functionPointer == 0 )
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID, functionPointer, true, nullptr);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Unregisters a C function as available for calling as a remote procedure call that was formerly registered
// with RegisterAsRemoteProcedureCall
//
// Parameters:
// uniqueID: A null terminated string to identify this procedure.  Must match the parameter
// passed to RegisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::UnregisterAsRemoteProcedureCall( RPCID  uniqueID )
{
	if ( uniqueID == 0
#if RPCID_STRING
			|| uniqueID[ 0 ] == 0
#endif
			)
		return;

// Don't call this while running because if you remove RPCs and add them they will not match the indices on the other systems anymore

	RakAssert(IsActive()==false);
	//RakAssert( strlen( uniqueID ) < 256 );


	rpcMap.RemoveNode(uniqueID);

	/*
	char uppercaseUniqueID[ 256 ];

	strcpy( uppercaseUniqueID, uniqueID );

	int counter = 0;

	while ( uniqueID[ counter ] )
	{
		uppercaseUniqueID[ counter ] = ( char ) toupper( uniqueID[ counter ] );
		counter++;
	}

	uppercaseUniqueID[ counter ] = 0;

	// Unique ID must exist

	RakAssert( rpcTree.IsIn( RPCNode( uppercaseUniqueID, 0 ) ) == true );


	rpcTree.Del( RPCNode( uppercaseUniqueID, 0 ) );
	*/
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Calls a C function on the server that the server already registered using RegisterAsRemoteProcedureCall
// If you want that function to return data you should call RPC from that system in the same way
// Returns true on a successful packet send (this does not indicate the recipient performed the call), false on failure
//
// Parameters:
// uniqueID: A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameter
// data: The block of data to send
// length: The size in BITS of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingChannel: When using ordered or sequenced packets, what channel to order these on.
// broadcast - Send this packet to everyone.
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to. Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// networkID: For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
// replyFromTarget: If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::RPC( RPCID  uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	RakAssert( uniqueID );
#if RPCID_STRING
	RakAssert( uniqueID[ 0 ] );
#endif
	RakAssert(orderingChannel >=0 && orderingChannel < 32);

	if ( uniqueID == 0 )
		return false;

#if RPCID_STRING
	if ( strlen( uniqueID ) > 256 )
	{

		RakAssert( 0 );

		return false; // Unique ID is too long
	}
#endif
	if (replyFromTarget && blockOnRPCReply==true)
	{
		// TODO - this should be fixed eventually
		// Prevent a bug where function A calls B (blocking) which calls C back on the sender, which calls D, and C is blocking.
		// blockOnRPCReply is a shared variable so making it unset would unset both blocks, rather than the lowest on the callstack
		// Fix by tracking which function the reply is for.
		return false;
	}

	unsigned *sendList;
//	bool callerAllocationDataUsed;
	unsigned sendListSize;

	// All this code modifies bcs->data and bcs->numberOfBitsToSend in order to transform an RPC request into an actual packet for SendImmediate
#if RPCID_STRING
	RPCIndex rpcIndex; // Index into the list of RPC calls so we know what number to encode in the packet
#endif
//	char *userData; // RPC ID (the name of it) and a pointer to the data sent by the user
//	int extraBuffer; // How many data bytes were allocated to hold the RPC header
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
//	int dataBlockAllocationLength; // Total number of bytes to allocate for the packet
//	char *writeTarget; // Used to hold either a block of allocated data or the externally allocated data

	sendListSize=0;
	bool routeSend;
	routeSend=false;

	if (broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( playerId, false );
		if (remoteSystemIndex!=(unsigned)-1 &&
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
		else if (router)
			routeSend=true;
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
		sendList = new unsigned[maximumNumberOfPeers];
#endif

		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].isActive && remoteSystemList[ remoteSystemIndex ].playerId != playerId )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0 && routeSend==false)
	{
#if defined(_COMPATIBILITY_1)
		delete [] sendList;
#endif

		return false;
	}
	if (routeSend)
		sendListSize=1;

	RakNet::BitStream outgoingBitStream;
	// remoteSystemList in network thread
	for (sendListIndex=0; sendListIndex < (unsigned)sendListSize; sendListIndex++)
	{
		outgoingBitStream.ResetWritePointer(); // Let us write at the start of the data block, rather than at the end

		if (shiftTimestamp)
		{
			outgoingBitStream.Write((unsigned char) ID_TIMESTAMP);
			outgoingBitStream.Write(RakNet::GetTime());
		}
		outgoingBitStream.Write((unsigned char) ID_RPC);
#if RPCID_STRING
		if (routeSend)
			rpcIndex=UNDEFINED_RPC_INDEX;
		else
			rpcIndex=remoteSystemList[sendList[sendListIndex]].rpcMap.GetIndexFromFunctionName(uniqueID); // Lots of trouble but we can only use remoteSystem->[whatever] in this thread so that is why this command was buffered
		if (rpcIndex!=UNDEFINED_RPC_INDEX)
		{
			// We have an RPC name to an index mapping, so write the index
			outgoingBitStream.Write(false);
			outgoingBitStream.WriteCompressed(rpcIndex);
		}
		else
		{
			// No mapping, so write the encoded RPC name
			outgoingBitStream.Write(true);
			stringCompressor->EncodeString(uniqueID, 256, &outgoingBitStream);
		}
#else
		outgoingBitStream.Write(uniqueID);
#endif
#if !RAKNET_LEGACY
		outgoingBitStream.Write((bool) ((replyFromTarget!=0)==true));
#endif
		outgoingBitStream.WriteCompressed( bitLength );
#if !RAKNET_LEGACY
		if (networkID==UNASSIGNED_NETWORK_ID)
		{
			// No object ID
			outgoingBitStream.Write(false);
		}
		else
		{
			// Encode an object ID.  This will use pointer to class member RPC
			outgoingBitStream.Write(true);
			outgoingBitStream.Write(networkID);
		}
#endif

		if ( bitLength > 0 )
			outgoingBitStream.WriteBits( (const unsigned char *) data, bitLength, false ); // Last param is false to write the raw data originally from another bitstream, rather than shifting from user data
		else
			outgoingBitStream.WriteCompressed( ( unsigned int ) 0 );

		if (routeSend)
			router->Send((const char*)outgoingBitStream.GetData(), outgoingBitStream.GetNumberOfBitsUsed(), priority,reliability,orderingChannel,playerId);
		else
			Send(&outgoingBitStream, priority, reliability, orderingChannel, remoteSystemList[sendList[sendListIndex]].playerId, false);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	if (replyFromTarget)
	{
		blockOnRPCReply=true;
		// 04/20/06 Just do this transparently.
		// We have to be able to read blocking packets out of order.  Otherwise, if two systems were to send blocking RPC calls to each other at the same time,
		// and they also had ordered packets waiting before the block, it would be impossible to unblock.
		// RakAssert(reliability==RELIABLE || reliability==UNRELIABLE);
		replyFromTargetBS=replyFromTarget;
		replyFromTargetPlayer=playerId;
		replyFromTargetBroadcast=broadcast;
	}

	// Do not enter this loop on blockOnRPCReply because it is a global which could be set to true by an RPC higher on the callstack, where one RPC was called while waiting for another RPC
	if (replyFromTarget)
//	if (blockOnRPCReply)
	{
//		Packet *p;
		RakNetTime stopWaitingTime;
//		RPCIndex arrivedRPCIndex;
//		char uniqueIdentifier[256];
		if (reliability == UNRELIABLE) {
			if (playerId == UNASSIGNED_PLAYER_ID)
				stopWaitingTime = RakNet::GetTime() + 1500; // Lets guess the ave. ping is 500.  Not important to be very accurate
			else
				stopWaitingTime = RakNet::GetTime() + GetAveragePing(playerId) * 3;
		}

		// For reliable messages, block until we get a reply or the connection is lost
		// For unreliable messages, block until we get a reply, the connection is lost, or 3X the ping passes
		while (blockOnRPCReply &&
			((reliability==RELIABLE || reliability==RELIABLE_ORDERED || reliability==RELIABLE_SEQUENCED) ||
			RakNet::GetTime() < stopWaitingTime))
		{

			RakSleep(30);

			if (routeSend==false && ValidSendTarget(playerId, broadcast)==false)
				return false;

			// I might not support processing other RPCs while blocking on one due to complexities I can't control
			// Problem is FuncA calls FuncB which calls back to the sender FuncC. Sometimes it is desirable to call FuncC before returning a return value
			// from FuncB - sometimes not.  There is also a problem with recursion where FuncA calls FuncB which calls FuncA - sometimes valid if
			// a different control path is taken in FuncA. (This can take many different forms)
			/*
			// Same as Receive, but doesn't automatically do RPCs
			p = ReceiveIgnoreRPC();
			if (p)
			{
				// Process all RPC calls except for those calling the function we are currently blocking in (to prevent recursion).
				if ( p->data[ 0 ] == ID_RPC )
				{
					RakNet::BitStream temp((unsigned char *) p->data, p->length, false);
					RPCNode *rpcNode;
					temp.IgnoreBits(8);
					bool nameIsEncoded;
					temp.Read(nameIsEncoded);
					if (nameIsEncoded)
					{
						stringCompressor->DecodeString((char*)uniqueIdentifier, 256, &temp);
					}
					else
					{
						temp.ReadCompressed( arrivedRPCIndex );
						rpcNode=rpcMap.GetNodeFromIndex( arrivedRPCIndex );
						if (rpcNode==0)
						{
							// Invalid RPC format

							RakAssert(0);

							DeallocatePacket(p);
							continue;
						}
						else
							strcpy(uniqueIdentifier, rpcNode->uniqueIdentifier);
					}

					if (strcmp(uniqueIdentifier, uniqueID)!=0)
					{
						HandleRPCPacket( ( char* ) p->data, p->length, p->playerId );
						DeallocatePacket(p);
					}
					else
					{
						PushBackPacket(p, false);
					}
				}
				else
				{
					PushBackPacket(p, false);
				}
			}
			*/
		}

		blockOnRPCReply=false;
	}

	return true;	
}


#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::RPC( RPCID  uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	if (bitStream)
		return RPC(uniqueID, (const char*) bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget);
	else
		return RPC(uniqueID, 0,0, priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
//
// Parameters:
// target: Which connection to close
// sendDisconnectionNotification: True to send ID_DISCONNECTION_NOTIFICATION to the recipient. False to close it silently.
// channel: If blockDuration > 0, the disconnect packet will be sent on this channel
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::CloseConnection( const PlayerID target, bool sendDisconnectionNotification, unsigned char orderingChannel )
{
	CloseConnectionInternal(target, sendDisconnectionNotification, false, orderingChannel);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
//
// Parameters
// playerId - The playerID to search for
//
// Returns
// An integer from 0 to the maximum number of peers -1, or -1 if that player is not found
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetIndexFromPlayerID( const PlayerID playerId )
{
	return GetIndexFromPlayerID(playerId, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// This function is only useful for looping through all players.
//
// Parameters
// index - an integer between 0 and the maximum number of players allowed - 1.
//
// Returns
// A valid playerID or UNASSIGNED_PLAYER_ID if no such player at that index
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetPlayerIDFromIndex( int index )
{
	// remoteSystemList in user thread
	//if ( index >= 0 && index < remoteSystemListSize )
	if ( index >= 0 && index < maximumNumberOfPeers )
		if (remoteSystemList[ index ].connectMode==RakPeer::RemoteSystemStruct::CONNECTED) // Don't give the user players that aren't fully connected, since sends will fail
			return remoteSystemList[ index ].playerId;

	return UNASSIGNED_PLAYER_ID;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Bans an IP from connecting. Banned IPs persist between connections.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// milliseconds - how many ms for a temporary ban.  Use 0 for a permanent ban
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AddToBanList( const char *IP, RakNetTime milliseconds )
{
	unsigned index;
	RakNetTime time = RakNet::GetTime();

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return ;

	// If this guy is already in the ban list, do nothing
	index = 0;

	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		if ( strcmp( IP, banList[ index ]->IP ) == 0 )
		{
			// Already in the ban list.  Just update the time
			if (milliseconds==0)
				banList[ index ]->timeout=0; // Infinite
			else
				banList[ index ]->timeout=time+milliseconds;
			banListMutex.Unlock();
			return;
		}
	}

	banListMutex.Unlock();

	BanStruct *banStruct = new BanStruct;
	banStruct->IP = new char [ 16 ];
	if (milliseconds==0)
		banStruct->timeout=0; // Infinite
	else
		banStruct->timeout=time+milliseconds;
	strcpy( banStruct->IP, IP );
	banListMutex.Lock();
	banList.Insert( banStruct );
	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows a previously banned IP to connect.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveFromBanList( const char *IP )
{
	unsigned index;
	BanStruct *temp;

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return ;

	index = 0;
	temp=0;

	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		if ( strcmp( IP, banList[ index ]->IP ) == 0 )
		{
			temp = banList[ index ];
			banList[ index ] = banList[ banList.Size() - 1 ];
			banList.RemoveAtIndex( banList.Size() - 1 );
			break;
		}
	}

	banListMutex.Unlock();

	if (temp)
	{
		delete [] temp->IP;
		delete temp;
	}

}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows all previously banned IPs to connect.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearBanList( void )
{
	unsigned index;
	index = 0;
	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		delete [] banList[ index ]->IP;
		delete [] banList[ index ];
	}

	banList.Clear();

	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Determines if a particular IP is banned.
//
// Parameters
// IP - Complete dotted IP address
//
// Returns
// True if IP matches any IPs in the ban list, accounting for any wildcards.
// False otherwise.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsBanned( const char *IP )
{
	unsigned banListIndex, characterIndex;
	RakNetTime time;
	BanStruct *temp;

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return false;

	banListIndex = 0;

	if ( banList.Size() == 0 )
		return false; // Skip the mutex if possible

	time = RakNet::GetTime();

	banListMutex.Lock();

	while ( banListIndex < banList.Size() )
	{
		if (banList[ banListIndex ]->timeout>0 && banList[ banListIndex ]->timeout<time)
		{
			// Delete expired ban
			temp = banList[ banListIndex ];
			banList[ banListIndex ] = banList[ banList.Size() - 1 ];
			banList.RemoveAtIndex( banList.Size() - 1 );
			delete [] temp->IP;
			delete temp;
		}
		else
		{
			characterIndex = 0;

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
			while ( true )
			{
				if ( banList[ banListIndex ]->IP[ characterIndex ] == IP[ characterIndex ] )
				{
					// Equal characters

					if ( IP[ characterIndex ] == 0 )
					{
						banListMutex.Unlock();
						// End of the string and the strings match

						return true;
					}

					characterIndex++;
				}

				else
				{
					if ( banList[ banListIndex ]->IP[ characterIndex ] == 0 || IP[ characterIndex ] == 0 )
					{
						// End of one of the strings
						break;
					}

					// Characters do not match
					if ( banList[ banListIndex ]->IP[ characterIndex ] == '*' )
					{
						banListMutex.Unlock();

						// Domain is banned.
						return true;
					}

					// Characters do not match and it is not a *
					break;
				}
			}

			banListIndex++;
		}
	}

	banListMutex.Unlock();

	// No match found.
	return false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified connected system.
//
// Parameters:
// target - who to ping
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping( const PlayerID target )
{
	PingInternal(target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified unconnected system.
// The remote system, if it is Initialized, will respond with ID_PONG.
// The final ping time will be encoded in the following sizeof(RakNetTime) bytes.  (Default is 4 bytes - See __GET_TIME_64BIT in NetworkTypes.h
//
// Parameters:
// host: Either a dotted IP address or a domain name.  Can be 255.255.255.255 for LAN broadcast.
// remotePort: Which port to connect to on the remote machine.
// onlyReplyOnAcceptingConnections: Only request a reply if the remote system has open connections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections )
{
	if ( host == 0 )
		return;

//	if ( IsActive() == false )
//		return;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream( sizeof(unsigned char) + sizeof(RakNetTime) );
	if ( onlyReplyOnAcceptingConnections )
		bitStream.Write((unsigned char)ID_PING_OPEN_CONNECTIONS);
	else
		bitStream.Write((unsigned char)ID_PING);

	bitStream.Write(RakNet::GetTime());

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	// No timestamp for 255.255.255.255
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );




}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the average of all ping times read for a specified target
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetAveragePing( const PlayerID playerId )
{
	int sum, quantity;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	for ( sum = 0, quantity = 0; quantity < PING_TIMES_ARRAY_SIZE; quantity++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ quantity ].pingTime == 65535 )
			break;
		else
			sum += remoteSystem->pingAndClockDifferential[ quantity ].pingTime;
	}

	if ( quantity > 0 )
		return sum / quantity;
	else
		return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the last ping time read for the specific player or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLastPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	if ( remoteSystem->pingAndClockDifferentialWriteIndex == 0 )
		return remoteSystem->pingAndClockDifferential[ PING_TIMES_ARRAY_SIZE - 1 ].pingTime;
	else
		return remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex - 1 ].pingTime;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the lowest ping time read or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLowestPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	return remoteSystem->lowestPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Ping the remote systems every so often.  This is off by default
// This will work anytime
//
// Parameters:
// doPing - True to start occasional pings.  False to stop them.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetOccasionalPing( bool doPing )
{
	occasionalPing = doPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Which system you are referring to.  Pass the value returned by GetInternalID to refer to yourself
//
// Returns:
// The data passed to SetRemoteStaticData stored as a bitstream
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNet::BitStream * RakPeer::GetRemoteStaticData( const PlayerID playerId )
{
	if ( playerId == myPlayerId )
		return & localStaticData;

	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem )
		return &(remoteSystem->staticData);
	else
		return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Whose static data to change.  Use your own playerId to change your own static data
// data: a block of data to store
// length: The length of data in bytes
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetRemoteStaticData( const PlayerID playerId, const char *data, const int length )
{
	if ( playerId == myPlayerId )
	{
		localStaticData.Reset();

		if ( data && length > 0 )
			localStaticData.Write( data, length );
	}
	else
	{
		RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, true );

		if ( remoteSystem == 0 )
			return;

		remoteSystem->staticData.Reset();
		remoteSystem->staticData.Write( data, length );
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends your static data to the specified system.  This is automatically done on connection.
// You should call this when you change your static data.
// To send the static data of another system (such as relaying their data) you should do this normally with Send
//
// Parameters:
// target: Who to send your static data to.  Specify UNASSIGNED_PLAYER_ID to broadcast to all
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendStaticData( const PlayerID target )
{
	SendStaticDataInternal(target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Length should be under 400 bytes, as a security measure against flood attacks
// Sets the data to send with an  (LAN server discovery) /(offline ping) response
// See the Ping sample project for how this is used.
// data: a block of data to store, or 0 for none
// length: The length of data in bytes, or 0 for none
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetOfflinePingResponse( const char *data, const unsigned int length )
{
	RakAssert(length < 400);

	rakPeerMutexes[ offlinePingResponse_Mutex ].Lock();
	offlinePingResponse.Reset();

	if ( data && length > 0 )
		offlinePingResponse.Write( data, length );

	rakPeerMutexes[ offlinePingResponse_Mutex ].Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique PlayerID that represents you on the the network
// Note that unlike in previous versions, this is a struct and is not sequential
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetInternalID( void ) const
{
	return myPlayerId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique address identifier that represents you on the the network and is based on your external
// IP / port (the IP / port the specified player uses to communicate with you)
// Note that unlike in previous versions, this is a struct and is not sequential
//
// Parameters:
// target: Which remote system you are referring to for your external ID
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetExternalID( const PlayerID target ) const
{
	unsigned i;
	PlayerID inactiveExternalId;

	inactiveExternalId=UNASSIGNED_PLAYER_ID;

	// First check for active connection with this playerId
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	{
		if (remoteSystemList[ i ].playerId == target || target==UNASSIGNED_PLAYER_ID )
		{
			if ( remoteSystemList[ i ].isActive )
				return remoteSystemList[ i ].myExternalPlayerId;
			else
				inactiveExternalId=remoteSystemList[ i ].myExternalPlayerId;
		}
	}

	return inactiveExternalId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable packet
// \param[in] time Time, in MS
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetTimeoutTime( RakNetTime timeMS, const PlayerID target )
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( target, false, true );

	if ( remoteSystem != 0 )
		remoteSystem->reliabilityLayer.SetTimeoutTime(timeMS);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Change the MTU size in order to improve performance when sending large packets
// This can only be called when not connected.
// A too high of value will cause packets not to arrive at worst and be fragmented at best.
// A too low of value will split packets unnecessarily.
//
// Parameters:
// size: Set according to the following table:
// 1500. The largest Ethernet packet size
// This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches.
// 1492. The size PPPoE prefers.
// 1472. Maximum size to use for pinging. (Bigger packets are fragmented.)
// 1468. The size DHCP prefers.
// 1460. Usable by AOL if you don't have large email attachments, etc.
// 1430. The size VPN and PPTP prefer.
// 1400. Maximum size for AOL DSL.
// 576. Typical value to connect to dial-up ISPs. (Default)
//
// Returns:
// False on failure (we are connected).  True on success.  Maximum allowed size is MAXIMUM_MTU_SIZE
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SetMTUSize( int size )
{
	if ( IsActive() )
		return false;

	if ( size < 512 )
		size = 512;
	else if ( size > MAXIMUM_MTU_SIZE )
		size = MAXIMUM_MTU_SIZE;

	MTUSize = size;

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the current MTU size
//
// Returns:
// The MTU sized specified in SetMTUSize
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetMTUSize( void ) const
{
	return MTUSize;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the number of IP addresses we have
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int RakPeer::GetNumberOfAddresses( void )
{
#if !defined(_COMPATIBILITY_1)
	char ipList[ 10 ][ 16 ];
	memset( ipList, 0, sizeof( char ) * 16 * 10 );
	SocketLayer::Instance()->GetMyIP( ipList );

	int i = 0;

	while ( ipList[ i ][ 0 ] )
		i++;

	return i;
#else
	RakAssert(0);
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a PlayerID struct, returns the dotted IP address string this binaryAddress field represents
//
// Returns:
// Null terminated dotted IP address string.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::PlayerIDToDottedIP( const PlayerID playerId ) const
{
#if !defined(_COMPATIBILITY_1)
	in_addr in;
	in.s_addr = playerId.binaryAddress;
	return inet_ntoa( in );
#else
	RakAssert(0); // Not supported
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns an IP address at index 0 to GetNumberOfAddresses-1
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::GetLocalIP( unsigned int index )
{
#if !defined(_COMPATIBILITY_1)
	static char ipList[ 10 ][ 16 ];

	if ( index >= 10 )
		index = 9;

	memset( ipList, 0, sizeof( char ) * 16 * 10 );

	SocketLayer::Instance()->GetMyIP( ipList );

	return ipList[ index ];
#else
	RakAssert(0);
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allow or disallow connection responses from any IP. Normally this should be false, but may be necessary
// when connection to servers with multiple IP addresses
//
// Parameters:
// allow - True to allow this behavior, false to not allow.  Defaults to false.  Value persists between connections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AllowConnectionResponseIPMigration( bool allow )
{
	allowConnectionResponseIPMigration = allow;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends a one byte message ID_ADVERTISE_SYSTEM to the remote unconnected system.
// This will tell the remote system our external IP outside the LAN, and can be used for NAT punch through
//
// Requires:
// The sender and recipient must already be started via a successful call to Initialize
//
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength )
{
	if ( IsActive() == false )
		return ;

	if (host==0)
		return;

	// This is a security measure.  Don't send data longer than this value
	RakAssert(dataLength <= MAX_OFFLINE_DATA_LENGTH);
	RakAssert(dataLength>=0);

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream;
	bitStream.Write((unsigned char)ID_ADVERTISE_SYSTEM);
	if (dataLength>0)
		bitStream.Write(data, dataLength);
	else
		bitStream.Write((unsigned char)0); // Pad

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );



	/*
	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RequestedConnectionStruct *rcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	if (data && dataLength>0)
	{
		rcs->data=new char [dataLength];
		rcs->dataLength=(unsigned short)dataLength;
		memcpy(rcs->data, data, dataLength);
	}
	else
	{
		rcs->data=0;
		rcs->dataLength=0;
	}
	rcs->actionToTake=RequestedConnectionStruct::ADVERTISE_SYSTEM;
	requestedConnectionList.WriteUnlock();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
	*/
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Controls how often to return ID_DOWNLOAD_PROGRESS for large message downloads.
// ID_DOWNLOAD_PROGRESS is returned to indicate a new partial message chunk, roughly the MTU size, has arrived
// As it can be slow or cumbersome to get this notification for every chunk, you can set the interval at which it is returned.
// Defaults to 0 (never return this notification)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetSplitMessageProgressInterval(int interval)
{
	RakAssert(interval>=0);
	splitMessageProgressInterval=interval;
	for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		remoteSystemList[ i ].reliabilityLayer.SetSplitMessageProgressInterval(splitMessageProgressInterval);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set how long to wait before giving up on sending an unreliable message
// Useful if the network is clogged up.
// Set to 0 or less to never timeout.  Defaults to 0.
// timeoutMS How many ms to wait before simply not sending an unreliable message.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetUnreliableTimeout(RakNetTime timeoutMS)
{
	RakAssert(timeoutMS>=0);
	unreliableTimeout=timeoutMS;
	for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		remoteSystemList[ i ].reliabilityLayer.SetUnreliableTimeout(unreliableTimeout);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Enables or disables our tracking of bytes input to and output from the network.
// This is required to get a frequency table, which is used to generate a new compression layer.
// You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only track
// part of the values sent over the network.
// This value persists between connect calls and defaults to false (no frequency tracking)
//
// Parameters:
// doCompile - true to track bytes.  Defaults to false
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetCompileFrequencyTable( bool doCompile )
{
	trackFrequencyTable = doCompile;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the frequency of outgoing bytes into outputFrequencyTable
// The purpose is to save to file as either a master frequency table from a sample game session for passing to
// GenerateCompressionLayer(false)
// You should only call this when disconnected.
// Requires that you first enable data frequency tracking by calling SetCompileFrequencyTable(true)
//
// Parameters:
// outputFrequencyTable (out): The frequency of each corresponding byte
//
// Returns:
// Ffalse (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetOutgoingFrequencyTable( unsigned int outputFrequencyTable[ 256 ] )
{
	if ( IsActive() )
		return false;

	if ( trackFrequencyTable == false )
		return false;

	memcpy( outputFrequencyTable, frequencyTable, sizeof( unsigned int ) * 256 );

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Generates the compression layer from the input frequency table.
// You should call this twice - once with inputLayer as true and once as false.
// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true
// Calling this function when there is an existing layer will overwrite the old layer
// You should only call this when disconnected
//
// Parameters:
// inputFrequencyTable: The frequency table returned from GetSendFrequencyTable(...)
// inputLayer - Whether inputFrequencyTable represents incoming data from other systems (true) or outgoing data from this system (false)
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )
{
	if ( IsActive() )
		return false;

	DeleteCompressionLayer( inputLayer );

	if ( inputLayer )
	{
		inputTree = new DataStructures::HuffmanEncodingTree;
		inputTree->GenerateFromFrequencyTable( inputFrequencyTable );
	}

	else
	{
		outputTree = new DataStructures::HuffmanEncodingTree;
		outputTree->GenerateFromFrequencyTable( inputFrequencyTable );
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Deletes the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory
// You should only call this when disconnected
//
// Parameters:
// inputLayer - Specifies the corresponding compression layer generated by GenerateCompressionLayer.
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::DeleteCompressionLayer( bool inputLayer )
{
	if ( IsActive() )
		return false;

	if ( inputLayer )
	{
		if ( inputTree )
		{
			delete inputTree;
			inputTree = 0;
		}
	}

	else
	{
		if ( outputTree )
		{
			delete outputTree;
			outputTree = 0;
		}
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The compression ratio.  A low compression ratio is good.  Compression is for outgoing data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetCompressionRatio( void ) const
{
	if ( rawBytesSent > 0 )
	{
		return ( float ) compressedBytesSent / ( float ) rawBytesSent;
	}

	else
		return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetDecompressionRatio( void ) const
{
	if ( rawBytesReceived > 0 )
	{
		return ( float ) compressedBytesReceived / ( float ) rawBytesReceived;
	}

	else
		return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Attatches a Plugin interface to run code automatically on message receipt in the Receive call
//
// \param messageHandler Pointer to a plugin to attach
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AttachPlugin( PluginInterface *plugin )
{
	if (messageHandlerList.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)
	{
		messageHandlerList.Insert(plugin);
		plugin->OnAttach(this);
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Detaches a Plugin interface to run code automatically on message receipt
//
// \param messageHandler Pointer to a plugin to detach
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DetachPlugin( PluginInterface *plugin )
{
	if (plugin==0)
		return;

	unsigned int index;
	index = messageHandlerList.GetIndexOf(plugin);
	if (index!=MAX_UNSIGNED_LONG)
	{
		messageHandlerList[index]->OnDetach(this);
		// Unordered list so delete from end for speed
		messageHandlerList[index]=messageHandlerList[messageHandlerList.Size()-1];
		messageHandlerList.Del();
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
//
// packet The packet you want to push back.
// pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PushBackPacket( Packet *packet, bool pushAtHead)
{
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	RakAssert(packet);
	if (pushAtHead)
		packetPool.PushAtHead(packet);
	else
		packetPool.Push(packet);
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetRouterInterface( RouterInterface *routerInterface )
{
	router=routerInterface;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveRouterInterface( RouterInterface *routerInterface )
{
	if (router==routerInterface)
		router=0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Adds simulated ping and packet loss to the outgoing data flow.
// To simulate bi-directional ping and packet loss, you should call this on both the sender and the recipient, with half the total ping and maxSendBPS value on each.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
#ifndef _RELEASE
	if (remoteSystemList)
	{
		unsigned short i;
		for (i=0; i < maximumNumberOfPeers; i++)
		//for (i=0; i < remoteSystemListSize; i++)
			remoteSystemList[i].reliabilityLayer.ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
	}

	_maxSendBPS=maxSendBPS;
	_minExtraPing=minExtraPing;
	_extraPingVariance=extraPingVariance;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns if you previously called ApplyNetworkSimulator
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsNetworkSimulatorActive( void )
{
	return _maxSendBPS>0 || _minExtraPing>0 || _extraPingVariance>0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// For internal use
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RPCMap* RakPeer::GetRPCMap( const PlayerID playerId)
{
    if (playerId==UNASSIGNED_PLAYER_ID)
		return &rpcMap;
	else
	{
		RemoteSystemStruct *rss=GetRemoteSystemFromPlayerID(playerId, false, true);
		if (rss)
			return &(rss->rpcMap);
		else
			return 0;
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNetStatisticsStruct * RakPeer::GetStatistics( const PlayerID playerId )
{
	if (playerId==UNASSIGNED_PLAYER_ID)
	{
		bool firstWrite=false;
		static RakNetStatisticsStruct sum;
		RakNetStatisticsStruct *systemStats;
		// Return a crude sum
		for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].isActive)
			{
				systemStats=remoteSystemList[ i ].reliabilityLayer.GetStatistics();

				if (firstWrite==false)
					memcpy(&sum, systemStats, sizeof(RakNetStatisticsStruct));
				else
					sum+=*systemStats;

				firstWrite = true;
			}
		}
		return &sum;
	}
	else
	{
		RemoteSystemStruct * rss;
		rss = GetRemoteSystemFromPlayerID( playerId, false, false );
		if ( rss && endThreads==false )
			return rss->reliabilityLayer.GetStatistics();
	}	

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void RakPeer::RemoveFromRequestedConnectionsList( const PlayerID playerId )
{
	int i;
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Lock();

	for ( i = 0; i < ( int ) requestedConnectionsList.Size(); )
	{
		if ( requestedConnectionsList[ i ]->playerId == playerId )
		{
			delete requestedConnectionsList[ i ];
			requestedConnectionsList.Del( i );
			break;
		}
	}

	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Unlock();
}
*/

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetIndexFromPlayerID( const PlayerID playerId, bool calledFromNetworkThread )
{
	unsigned i;

	if ( playerId == UNASSIGNED_PLAYER_ID )
		return -1;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerId, &objectExists);
		if (objectExists)
		{
			RakAssert(remoteSystemList[remoteSystemLookup[index].index].playerId==playerId);
			return remoteSystemLookup[index].index;
		}
		else
			return -1;
	}
	else
	{
		// remoteSystemList in user and network thread
		for ( i = 0; i < maximumNumberOfPeers; i++ )
			if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].playerId == playerId )
				return i;
	}

	return -1;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SendConnectionRequest( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	// Already connected?
	if (GetRemoteSystemFromPlayerID(playerId, false, true))
		return false;

	RakAssert(passwordDataLength <= 256);

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	RequestedConnectionStruct *rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	rcs->data=0;
	rcs->actionToTake=RequestedConnectionStruct::CONNECT;
	memcpy(rcs->outgoingPassword, passwordData, passwordDataLength);
	rcs->outgoingPasswordLength=(unsigned char) passwordDataLength;
	requestedConnectionList.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::IPToPlayerID( const char* host, unsigned short remotePort, PlayerID *playerId )
{
	if ( host == 0 )
		return ;

	playerId->binaryAddress = inet_addr( host );

	playerId->port = remotePort;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RemoteSystemStruct *RakPeer::GetRemoteSystemFromPlayerID( const PlayerID playerID, bool calledFromNetworkThread, bool onlyActive ) const
{
	unsigned i;

	if ( playerID == UNASSIGNED_PLAYER_ID )
		return 0;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerID, &objectExists);
		if (objectExists)
		{

			RakAssert(remoteSystemList[ remoteSystemLookup[index].index ].playerId==playerID);

			return remoteSystemList + remoteSystemLookup[index].index;
		}
	}
	else
	{
		int deadConnectionIndex=-1;

		// Active connections take priority.  But if there are no active connections, return the first systemAddress match found
		for ( i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].playerId == playerID )
			{
				if ( remoteSystemList[ i ].isActive )
					return remoteSystemList + i;
				else if (deadConnectionIndex==-1)
					deadConnectionIndex=i;
			}
		}

		if (deadConnectionIndex!=-1 && onlyActive==false)
			return remoteSystemList + deadConnectionIndex;
	}

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ParseConnectionRequestPacket( RakPeer::RemoteSystemStruct *remoteSystem, PlayerID playerId, const char *data, int byteSize )
{
	// If we are full tell the sender.
	if ( !AllowIncomingConnections() )
	{
		unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
		// SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, ( char* ) & c, sizeof( char ), systemAddress.binaryAddress, systemAddress.port );
		SendImmediate(( char* ) & c, sizeof( char )*8, SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime());
		remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
	}
	else
	{
		const char *password = data + sizeof(unsigned char);
		int passwordLength = byteSize - sizeof(unsigned char);

		if ( incomingPasswordLength == passwordLength &&
			memcmp( password, incomingPassword, incomingPasswordLength ) == 0 )
		{
			remoteSystem->connectMode=RemoteSystemStruct::HANDLING_CONNECTION_REQUEST;

#if !defined(_COMPATIBILITY_1)
			if ( usingSecurity == false )
#endif
			{
#ifdef _TEST_AES
				unsigned char AESKey[ 16 ];
				// Save the AES key
				for ( i = 0; i < 16; i++ )
					AESKey[ i ] = i;

				OnConnectionRequest( remoteSystem, AESKey, true );
#else
				// Connect this player assuming we have open slots
				OnConnectionRequest( remoteSystem, 0, false );
#endif
			}
#if !defined(_COMPATIBILITY_1)
			else
				SecuredConnectionResponse( playerId );
#endif
		}
		else
		{
			// This one we only send once since we don't care if it arrives.
			unsigned char c = ID_INVALID_PASSWORD;
			// SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, ( char* ) & c, sizeof( char ), systemAddress.binaryAddress, systemAddress.port );
			SendImmediate(( char* ) & c, sizeof( char )*8, SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime());
			remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
		}
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::OnConnectionRequest( RakPeer::RemoteSystemStruct *remoteSystem, unsigned char *AESKey, bool setAESKey )
{
	// Already handled by caller
	//if ( AllowIncomingConnections() )
	{
#ifdef __USE_IO_COMPLETION_PORTS
		unsigned index;

		// remoteSystemList in network thread
		for ( index = 0; index < maximumNumberOfPeers; index++ )
		//for ( index = 0; index < remoteSystemListSize; index++ )
			if ( remoteSystemList + index == remoteSystem )
				break;

		if ( SetupIOCompletionPortSocket( index ) == false )
		{
			// Socket error
			RakAssert( 0 );
			return ;
		}
#endif

		RakNet::BitStream bitStream(sizeof(unsigned char)+sizeof(unsigned short)+sizeof(unsigned int)+sizeof(unsigned short)+sizeof(PlayerIndex));
		bitStream.Write((unsigned char)ID_CONNECTION_REQUEST_ACCEPTED);
//#ifdef __USE_IO_COMPLETION_PORTS
//		bitStream.Write((unsigned short)myPlayerId.port + ( unsigned short ) index + ( unsigned short ) 1);
//#else
//		bitStream.Write((unsigned short)myPlayerId.port);
//#endif
		bitStream.Write(remoteSystem->playerId.binaryAddress);
		bitStream.Write(remoteSystem->playerId.port);
		bitStream.Write(( PlayerIndex ) GetIndexFromPlayerID( remoteSystem->playerId, true ));
		bitStream.Write(SAMPRakNet::GetToken());


		SendImmediate((char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, remoteSystem->playerId, false, false, RakNet::GetTime());

		// Don't set secure connections immediately because we need the ack from the remote system to know ID_CONNECTION_REQUEST_ACCEPTED
		// As soon as a 16 byte packet arrives, we will turn on AES.  This works because all encrypted packets are multiples of 16 and the
		// packets I happen to be sending are less than 16 bytes
		remoteSystem->setAESKey=setAESKey;
		if ( setAESKey )
		{
			memcpy(remoteSystem->AESKey, AESKey, 16);
			remoteSystem->connectMode=RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET;
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::NotifyAndFlagForDisconnect( const PlayerID playerId, bool performImmediate, unsigned char orderingChannel )
{
	RakNet::BitStream temp( sizeof(unsigned char) );
	temp.Write( (unsigned char) ID_DISCONNECTION_NOTIFICATION );
	if (performImmediate)
	{
		SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), LOW_PRIORITY, RELIABLE_ORDERED, orderingChannel, playerId, false, false, RakNet::GetTime());
		RemoteSystemStruct *rss=GetRemoteSystemFromPlayerID(playerId, true, true);
		rss->connectMode=RemoteSystemStruct::DISCONNECT_ASAP;
	}
	else
	{
		SendBuffered((const char*)temp.GetData(), temp.GetNumberOfBitsUsed(), LOW_PRIORITY, RELIABLE_ORDERED, orderingChannel, playerId, false, RemoteSystemStruct::DISCONNECT_ASAP);
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetNumberOfRemoteInitiatedConnections( void ) const
{
	unsigned short i, numberOfIncomingConnections;

	if ( remoteSystemList == 0 || endThreads == true )
		return 0;

	numberOfIncomingConnections = 0;

	// remoteSystemList in network thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].weInitiatedTheConnection == false && remoteSystemList[i].connectMode==RemoteSystemStruct::CONNECTED)
			numberOfIncomingConnections++;
	}

	return numberOfIncomingConnections;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RemoteSystemStruct * RakPeer::AssignPlayerIDToRemoteSystemList( const PlayerID playerId, RemoteSystemStruct::ConnectMode connectionMode )
{
	RemoteSystemStruct * remoteSystem;
	unsigned i,j;
	RakNetTime time = RakNet::GetTime();

	RakAssert(playerId!=UNASSIGNED_PLAYER_ID);


	// remoteSystemList in user thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive==false )
		{
			remoteSystem=remoteSystemList+i;
			remoteSystem->rpcMap.Clear();
			remoteSystem->playerId = playerId;
			remoteSystem->isActive=true; // This one line causes future incoming packets to go through the reliability layer
			remoteSystem->reliabilityLayer.SetSplitMessageProgressInterval(splitMessageProgressInterval);
			remoteSystem->reliabilityLayer.SetUnreliableTimeout(unreliableTimeout);
			remoteSystem->reliabilityLayer.SetEncryptionKey( 0 );

			for ( j = 0; j < (unsigned) PING_TIMES_ARRAY_SIZE; j++ )
			{
				remoteSystem->pingAndClockDifferential[ j ].pingTime = 65535;
				remoteSystem->pingAndClockDifferential[ j ].clockDifferential = 0;
			}

			remoteSystem->connectMode=connectionMode;
			remoteSystem->pingAndClockDifferentialWriteIndex = 0;
			remoteSystem->lowestPing = 65535;
			remoteSystem->nextPingTime = 0; // Ping immediately
			remoteSystem->weInitiatedTheConnection = false;
			remoteSystem->staticData.Reset();
			remoteSystem->connectionTime = time;
			remoteSystem->myExternalPlayerId = UNASSIGNED_PLAYER_ID;
			remoteSystem->setAESKey=false;
			remoteSystem->lastReliableSend=time;

			// Reserve this reliability layer for ourselves.
			remoteSystem->reliabilityLayer.Reset(true);

			/// Add this player to the lookup tree
			PlayerIDAndIndex playerIDAndIndex;
			playerIDAndIndex.playerId=playerId;
			playerIDAndIndex.index=i;
			remoteSystemLookup.Insert(playerId,playerIDAndIndex);

			return remoteSystem;
		}
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Adjust the first four bytes (treated as unsigned int) of the pointer
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ShiftIncomingTimestamp( unsigned char *data, PlayerID playerId ) const
{
#ifdef _DEBUG
	RakAssert( IsActive() );
	RakAssert( data );
#endif

	RakNet::BitStream timeBS( data, sizeof(RakNetTime), false);
	RakNetTime encodedTimestamp;
	timeBS.Read(encodedTimestamp);

	encodedTimestamp = encodedTimestamp - GetBestClockDifferential( playerId );
	timeBS.SetWriteOffset(0);
	timeBS.Write(encodedTimestamp);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
RakNetTime RakPeer::GetBestClockDifferential( const PlayerID playerId ) const
{
	int counter, lowestPingSoFar;
	RakNetTime clockDifferential;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, true, true );

	if ( remoteSystem == 0 )
		return 0;

	lowestPingSoFar = 65535;

	clockDifferential = 0;

	for ( counter = 0; counter < PING_TIMES_ARRAY_SIZE; counter++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime == 65535 )
			break;

		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime < lowestPingSoFar )
		{
			clockDifferential = remoteSystem->pingAndClockDifferential[ counter ].clockDifferential;
			lowestPingSoFar = remoteSystem->pingAndClockDifferential[ counter ].pingTime;
		}
	}

	return clockDifferential;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Handles an RPC packet.  If you get a packet with the ID ID_RPC you should pass it to this function
// This is already done in Multiplayer.cpp, so if you use the Multiplayer class it is handled for you.
//
// Parameters:
// packet - A packet returned from Receive with the ID ID_RPC
//
// Returns:
// true on success, false on a bad packet or an unregistered function
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::HandleRPCPacket( const char *data, int length, PlayerID playerId )
{
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// Unique identifier string length - unsigned char
	// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
	// Number of bits of the data (int)
	// The data

	RakNet::BitStream incomingBitStream( (unsigned char *) data, length, false );
#if RPCID_STRING
	char uniqueIdentifier[ 256 ];
#else
	RPCID uniqueIdentifier;
#endif
//	unsigned int bitLength;
	unsigned char *userData;
	//bool hasTimestamp;
	bool nameIsEncoded, networkIDIsEncoded;
	RPCIndex rpcIndex;
	RPCNode *node;
	RPCParameters rpcParms;
	NetworkID networkID;
	bool blockingCommand;
	RakNet::BitStream replyToSender;
	rpcParms.replyToSender=&replyToSender;

	rpcParms.recipient=this;
	rpcParms.sender=playerId;

	// Note to self - if I change this format then I have to change the PacketLogger class too
	incomingBitStream.IgnoreBits(8);
	if (data[0]==ID_TIMESTAMP)
		incomingBitStream.IgnoreBits(8*(sizeof(RakNetTime)+sizeof(unsigned char)));
	
#if RPCID_STRING
	if ( incomingBitStream.Read( nameIsEncoded ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}

	if (nameIsEncoded)
	{
		if ( stringCompressor->DecodeString(uniqueIdentifier, 256, &incomingBitStream) == false )
		{

			RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

			return false;
		}

		rpcIndex = rpcMap.GetIndexFromFunctionName(uniqueIdentifier);
	}
	else
	{
		if ( incomingBitStream.ReadCompressed( rpcIndex ) == false )
		{

			RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

			return false;
		}
	}
#else
	nameIsEncoded = false;
	if ( incomingBitStream.Read( uniqueIdentifier ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}
	rpcIndex = rpcMap.GetIndexFromFunctionName(uniqueIdentifier);
#endif

#if RAKNET_LEGACY
	blockingCommand = false;
#else
	if ( incomingBitStream.Read( blockingCommand ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}
#endif

	/*
	if ( incomingBitStream.Read( rpcParms.hasTimestamp ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}
	*/

	if ( incomingBitStream.ReadCompressed( rpcParms.numberOfBitsOfData ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}

	networkID = { { 0 } };
#if RAKNET_LEGACY
	networkIDIsEncoded = false;
#else
	if ( incomingBitStream.Read( networkIDIsEncoded ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

		return false;
	}

	if (networkIDIsEncoded)
	{
		if ( incomingBitStream.Read( networkID ) == false )
		{

			RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error

			return false;
		}
	}
#endif

	if (rpcIndex==UNDEFINED_RPC_INDEX)
	{
		// Unregistered function
		RakAssert(0);
		return false;
	}

	node = rpcMap.GetNodeFromIndex(rpcIndex);
	if (node==0)
	{

		RakAssert( 0 ); // Should never happen except perhaps from threading errors?  No harm in checking anyway

		return false;
	}

	// Make sure the call type matches - if this is a pointer to a class member then networkID must be defined.  Otherwise it must not be defined
	if (node->isPointerToMember==true && networkIDIsEncoded==false)
	{
		// If this hits then this pointer was registered as a class member function but the packet does not have an NetworkID.
		// Most likely this means this system registered a function with REGISTER_CLASS_MEMBER_RPC and the remote system called it
		// using the unique ID for a function registered with REGISTER_STATIC_RPC.
		RakAssert(0);
		return false;
	}

	if (node->isPointerToMember==false && networkIDIsEncoded==true)
	{
		// If this hits then this pointer was not registered as a class member function but the packet does have an NetworkID.
		// Most likely this means this system registered a function with REGISTER_STATIC_RPC and the remote system called it
		// using the unique ID for a function registered with REGISTER_CLASS_MEMBER_RPC.
		RakAssert(0);
		return false;
	}

#if RPCID_STRING
	if (nameIsEncoded && GetRemoteSystemFromPlayerID(playerId, false, true))
	{
		// Send ID_RPC_MAPPING to the sender so they know what index to use next time
		RakNet::BitStream rpcMapBitStream;
		rpcMapBitStream.Write((unsigned char)ID_RPC_MAPPING);
		stringCompressor->EncodeString(node->uniqueIdentifier, 256, &rpcMapBitStream);
        rpcMapBitStream.WriteCompressed(rpcIndex);
		SendBuffered( (const char*)rpcMapBitStream.GetData(), rpcMapBitStream.GetNumberOfBitsUsed(), HIGH_PRIORITY, UNRELIABLE, 0, playerId, false, RemoteSystemStruct::NO_ACTION );
	}
#endif

	// Call the function
	if ( rpcParms.numberOfBitsOfData == 0 )
	{
		rpcParms.input=0;
#if !RAKNET_LEGACY
		if (networkIDIsEncoded)
		{
			void *object = NetworkIDGenerator::GET_OBJECT_FROM_ID(networkID);
			if (object)
				(node->memberFunctionPointer(object, &rpcParms));
		}
		else
#endif
		{
			node->staticFunctionPointer( &rpcParms, node->extraPointer );
		}
	}
	else
	{
		if ( incomingBitStream.GetNumberOfUnreadBits() == 0 )
		{

			RakAssert( 0 );

			return false; // No data was appended!
		}

		// We have to copy into a new data chunk because the user data might not be byte aligned.
		bool usedAlloca=false;
#if !defined(_COMPATIBILITY_1)
		if (BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) < MAX_ALLOCA_STACK_ALLOCATION)
		{
			userData = ( unsigned char* ) alloca( BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) );
			usedAlloca=true;
		}
		else
#endif
			userData = new unsigned char[BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits())];


		// The false means read out the internal representation of the bitstream data rather than
		// aligning it as we normally would with user data.  This is so the end user can cast the data received
		// into a bitstream for reading
		if ( incomingBitStream.ReadBits( ( unsigned char* ) userData, rpcParms.numberOfBitsOfData, false ) == false )
		{

			RakAssert( 0 );

			#if defined(_COMPATIBILITY_1)
			delete [] userData;
			#endif

			return false; // Not enough data to read
		}

//		if ( rpcParms.hasTimestamp )
//			ShiftIncomingTimestamp( userData, playerId );

		// Call the function callback
		rpcParms.input=userData;
#if !RAKNET_LEGACY
		if (networkIDIsEncoded)
		{
			void *object = NetworkIDGenerator::GET_OBJECT_FROM_ID(networkID);
			if (object)
				(node->memberFunctionPointer(object, &rpcParms));
		}
		else
#endif
		{
			node->staticFunctionPointer( &rpcParms, node->extraPointer );
		}


		if (usedAlloca==false)
			delete [] userData;
	}

#if !RAKNET_LEGACY
	if (blockingCommand)
	{
		RakNet::BitStream reply;
		reply.Write((unsigned char) ID_RPC_REPLY);
		reply.Write((char*)replyToSender.GetData(), replyToSender.GetNumberOfBytesUsed());
		Send(&reply, HIGH_PRIORITY, RELIABLE, 0, playerId, false);
	}
#endif

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
* Handles an RPC reply packet.  This is data returned from an RPC call
*
* \param data A packet returned from Receive with the ID ID_RPC
* \param length The size of the packet data
* \param playerId The sender of the packet
*
* \return true on success, false on a bad packet or an unregistered function
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::HandleRPCReplyPacket( const char *data, int length, PlayerID playerId )
{
	if (blockOnRPCReply)
	{
		if ((playerId==replyFromTargetPlayer && replyFromTargetBroadcast==false) ||
			(playerId!=replyFromTargetPlayer && replyFromTargetBroadcast==true))
		{
			replyFromTargetBS->Write(data+1, length-1);
			blockOnRPCReply=false;
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef __USE_IO_COMPLETION_PORTS
bool RakPeer::SetupIOCompletionPortSocket( int index )
{
	SOCKET newSocket;

	if ( remoteSystemList[ index ].reliabilityLayer.GetSocket() != INVALID_SOCKET )
		closesocket( remoteSystemList[ index ].reliabilityLayer.GetSocket() );

	newSocket = SocketLayer::Instance()->CreateBoundSocket( myPlayerId.port + index + 1, false );

	SocketLayer::Instance()->Connect( newSocket, remoteSystemList[ index ].playerId.binaryAddress, remoteSystemList[ index ].playerId.port ); // port is the port of the client

	remoteSystemList[ index ].reliabilityLayer.SetSocket( newSocket );

	// Associate our new socket with a completion port and do the first read
	return SocketLayer::Instance()->AssociateSocketWithCompletionPortAndRead( newSocket, remoteSystemList[ index ].playerId.binaryAddress, remoteSystemList[ index ].playerId.port, this );
}

#endif

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GenerateSYNCookieRandomNumber( void )
{
#if !defined(_COMPATIBILITY_1)
	unsigned int number;
	int i;
	memcpy( oldRandomNumber, newRandomNumber, sizeof( newRandomNumber ) );

	for ( i = 0; i < (int) sizeof( newRandomNumber ); i += (int) sizeof( number ) )
	{
		number = randomMT();
		memcpy( newRandomNumber + i, ( char* ) & number, sizeof( number ) );
	}

	randomNumberExpirationTime = RakNet::GetTime() + SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SecuredConnectionResponse( const PlayerID playerId )
{
#if !defined(_COMPATIBILITY_1)
	CSHA1 sha1;
	RSA_BIT_SIZE n;
	big::u32 e;
	unsigned char connectionRequestResponse[ 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20 ];
	connectionRequestResponse[ 0 ] = ID_SECURED_CONNECTION_RESPONSE;

	if ( randomNumberExpirationTime < RakNet::GetTime() )
		GenerateSYNCookieRandomNumber();

	// Hash the SYN-Cookie
	// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
	sha1.Reset();
	sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
	sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
	sha1.Update( ( unsigned char* ) & ( newRandomNumber ), 20 );
	sha1.Final();

	// Write the cookie
	memcpy( connectionRequestResponse + 1, sha1.GetHash(), 20 );

	// Write the public keys
	rsacrypt.getPublicKey( e, n );
#ifdef HOST_ENDIAN_IS_BIG
	// Mangle the keys on a Big-endian machine before sending
	BSWAPCPY( (unsigned char *)(connectionRequestResponse + 1 + 20),
		(unsigned char *)&e, sizeof( big::u32 ) );
	BSWAPCPY( (unsigned char *)(connectionRequestResponse + 1 + 20 + sizeof( big::u32 ) ),
		(unsigned char *)n, sizeof( RSA_BIT_SIZE ) );
#else
	memcpy( connectionRequestResponse + 1 + 20, ( char* ) & e, sizeof( big::u32 ) );
	memcpy( connectionRequestResponse + 1 + 20 + sizeof( big::u32 ), n, sizeof( RSA_BIT_SIZE ) );
#endif

	// s2c public key, syn-cookie
	//SocketLayer::Instance()->SendTo( connectionSocket, ( char* ) connectionRequestResponse, 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20, playerId.binaryAddress, playerId.port );
	// All secure connection requests are unreliable because the entire process needs to be restarted if any part fails.
	// Connection requests are resent periodically
	SendImmediate(( char* ) connectionRequestResponse, (1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20) *8, SYSTEM_PRIORITY, UNRELIABLE, 0, playerId, false, false, RakNet::GetTime());
#endif
}

void RakPeer::SecuredConnectionConfirmation( RakPeer::RemoteSystemStruct * remoteSystem, char* data )
{
#if !defined(_COMPATIBILITY_1)
	int i, j;
	unsigned char randomNumber[ 20 ];
	unsigned int number;
	//bool doSend;
	Packet *packet;
	big::u32 e;
	RSA_BIT_SIZE n, message, encryptedMessage;
	big::RSACrypt<RSA_BIT_SIZE> privKeyPncrypt;

	// Make sure that we still want to connect
	if (remoteSystem->connectMode!=RemoteSystemStruct::REQUESTED_CONNECTION)
		return;

	// Copy out e and n
#ifdef HOST_ENDIAN_IS_BIG
	BSWAPCPY( (unsigned char *)&e, (unsigned char *)(data + 1 + 20), sizeof( big::u32 ) );
	BSWAPCPY( (unsigned char *)n, (unsigned char *)(data + 1 + 20 + sizeof( big::u32 )), sizeof( RSA_BIT_SIZE ) );
#else
	memcpy( ( char* ) & e, data + 1 + 20, sizeof( big::u32 ) );
	memcpy( n, data + 1 + 20 + sizeof( big::u32 ), sizeof( RSA_BIT_SIZE ) );
#endif

	// If we preset a size and it doesn't match, or the keys do not match, then tell the user
	if ( usingSecurity == true && keysLocallyGenerated == false )
	{
		if ( memcmp( ( char* ) & e, ( char* ) & publicKeyE, sizeof( big::u32 ) ) != 0 ||
			memcmp( n, publicKeyN, sizeof( RSA_BIT_SIZE ) ) != 0 )
		{
			packet=AllocPacket(1);
			packet->data[ 0 ] = ID_RSA_PUBLIC_KEY_MISMATCH;
			packet->bitSize = sizeof( char ) * 8;
			packet->playerId = remoteSystem->playerId;
			packet->playerIndex = ( PlayerIndex ) GetIndexFromPlayerID( packet->playerId, true );
			AddPacketToProducer(packet);
			remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
			return;
		}
	}

	// Create a random number
	for ( i = 0; i < (int) sizeof( randomNumber ); i += (int) sizeof( number ) )
	{
		number = randomMT();
		memcpy( randomNumber + i, ( char* ) & number, sizeof( number ) );
	}

	memset( message, 0, sizeof( message ) );
	RakAssert( sizeof( message ) >= sizeof( randomNumber ) );

#ifdef HOST_ENDIAN_IS_BIG
	// Scramble the plaintext message
	BSWAPCPY( (unsigned char *)message, randomNumber, sizeof(randomNumber) );
#else
	memcpy( message, randomNumber, sizeof( randomNumber ) );
#endif
	privKeyPncrypt.setPublicKey( e, n );
	privKeyPncrypt.encrypt( message, encryptedMessage );
#ifdef HOST_ENDIAN_IS_BIG
	// A big-endian machine needs to scramble the byte order of an outgoing (encrypted) message
	BSWAPSELF( (unsigned char *)encryptedMessage, sizeof( RSA_BIT_SIZE ) );
#endif

	/*
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Lock();
	for ( i = 0; i < ( int ) requestedConnectionsList.Size(); i++ )
	{
		if ( requestedConnectionsList[ i ]->playerId == playerId )
		{
			doSend = true;
			// Generate the AES key

			for ( j = 0; j < 16; j++ )
				requestedConnectionsList[ i ]->AESKey[ j ] = data[ 1 + j ] ^ randomNumber[ j ];

			requestedConnectionsList[ i ]->setAESKey = true;

			break;
		}
	}
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Unlock();
	*/

	// Take the remote system's AESKey and XOR with our random number.
		for ( j = 0; j < 16; j++ )
			remoteSystem->AESKey[ j ] = data[ 1 + j ] ^ randomNumber[ j ];
	remoteSystem->setAESKey = true;

//	if ( doSend )
//	{
		char reply[ 1 + 20 + sizeof( RSA_BIT_SIZE ) ];
		// c2s RSA(random number), same syn-cookie
		reply[ 0 ] = ID_SECURED_CONNECTION_CONFIRMATION;
		memcpy( reply + 1, data + 1, 20 );  // Copy the syn-cookie
		memcpy( reply + 1 + 20, encryptedMessage, sizeof( RSA_BIT_SIZE ) ); // Copy the encoded random number

		//SocketLayer::Instance()->SendTo( connectionSocket, reply, 1 + 20 + sizeof( RSA_BIT_SIZE ), playerId.binaryAddress, playerId.port );
		// All secure connection requests are unreliable because the entire process needs to be restarted if any part fails.
		// Connection requests are resent periodically
		SendImmediate((char*)reply, (1 + 20 + sizeof( RSA_BIT_SIZE )) * 8, SYSTEM_PRIORITY, UNRELIABLE, 0, remoteSystem->playerId, false, false, RakNet::GetTime());
//	}

#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::AllowIncomingConnections(void) const
{
	return GetNumberOfRemoteInitiatedConnections() < GetMaximumIncomingConnections();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendStaticDataInternal( const PlayerID target, bool performImmediate )
{
	RakNet::BitStream reply( sizeof(unsigned char) + localStaticData.GetNumberOfBytesUsed() );
	reply.Write( (unsigned char) ID_RECEIVED_STATIC_DATA );
	reply.Write( (char*)localStaticData.GetData(), localStaticData.GetNumberOfBytesUsed() );

	if (performImmediate)
	{
		if ( target == UNASSIGNED_PLAYER_ID )
			SendImmediate( (char*)reply.GetData(), reply.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, target, true, false, RakNet::GetTime() );
		else
			SendImmediate( (char*)reply.GetData(), reply.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, target, false, false, RakNet::GetTime() );
	}
	else
	{
		if ( target == UNASSIGNED_PLAYER_ID )
			Send( &reply, SYSTEM_PRIORITY, RELIABLE, 0, target, true );
		else
			Send( &reply, SYSTEM_PRIORITY, RELIABLE, 0, target, false );
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PingInternal( const PlayerID target, bool performImmediate )
{
	if ( IsActive() == false )
		return ;

	RakNet::BitStream bitStream(sizeof(unsigned char)+sizeof(RakNetTime));
	bitStream.Write((unsigned char)ID_INTERNAL_PING);
	RakNetTimeNS currentTimeNS = RakNet::GetTimeNS();
	RakNetTime currentTime = RakNet::GetTime();
	bitStream.Write(currentTime);
	if (performImmediate)
		SendImmediate( (char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, UNRELIABLE, 0, target, false, false, currentTimeNS );
	else
		Send( &bitStream, SYSTEM_PRIORITY, UNRELIABLE, 0, target, false );
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::CloseConnectionInternal( const PlayerID target, bool sendDisconnectionNotification, bool performImmediate, unsigned char orderingChannel )
{
	unsigned i,j;


	RakAssert(orderingChannel >=0 && orderingChannel < 32);


	if (target==UNASSIGNED_PLAYER_ID)
		return;

	if ( remoteSystemList == 0 || endThreads == true )
		return;

	if (sendDisconnectionNotification)
	{
		NotifyAndFlagForDisconnect(target, performImmediate, orderingChannel);
	}
	else
	{
		if (performImmediate)
		{
			i = 0;
			// remoteSystemList in user thread
			for ( ; i < maximumNumberOfPeers; i++ )
				//for ( ; i < remoteSystemListSize; i++ )
			{
				if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].playerId == target )
				{
					// Found the index to stop
					remoteSystemList[ i ].isActive=false;

					// Reserve this reliability layer for ourselves
					//remoteSystemList[ i ].playerId = UNASSIGNED_PLAYER_ID;
					
					for (j=0; j < messageHandlerList.Size(); j++)
					{
						messageHandlerList[j]->OnCloseConnection(this, target);
					}

					// Clear any remaining messages
					remoteSystemList[ i ].reliabilityLayer.Reset(false);

					// Remove from the lookup list
					remoteSystemLookup.Remove(target);

					break;
				}
			}
		}
		else
		{
			BufferedCommandStruct *bcs;
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
			bcs=bufferedCommands.WriteLock();
			bcs->command=BufferedCommandStruct::BCS_CLOSE_CONNECTION;
			bcs->playerId=target;
			bcs->data=0;
			bcs->orderingChannel=orderingChannel;
			bufferedCommands.WriteUnlock();
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
		}
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::ValidSendTarget(PlayerID playerId, bool broadcast)
{
	unsigned remoteSystemIndex;

	// remoteSystemList in user thread.  This is slow so only do it in debug
	for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
	//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; remoteSystemIndex++ )
	{
		if ( remoteSystemList[ remoteSystemIndex ].isActive &&
			remoteSystemList[ remoteSystemIndex ].connectMode==RakPeer::RemoteSystemStruct::CONNECTED && // Not fully connected players are not valid user-send targets because the reliability layer wasn't reset yet
			( ( broadcast == false && remoteSystemList[ remoteSystemIndex ].playerId == playerId ) ||
			( broadcast == true && remoteSystemList[ remoteSystemIndex ].playerId != playerId ) )
			)
			return true;
	}

	return false;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendBuffered( const char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, RemoteSystemStruct::ConnectMode connectionMode )
{

	RakAssert(orderingChannel >=0 && orderingChannel < 32);



	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	bcs=bufferedCommands.WriteLock();
	bcs->data = new char[BITS_TO_BYTES(numberOfBitsToSend)]; // Making a copy doesn't lose efficiency because I tell the reliability layer to use this allocation for its own copy

	RakAssert(bcs->data);

	memcpy(bcs->data, data, BITS_TO_BYTES(numberOfBitsToSend));
    bcs->numberOfBitsToSend=numberOfBitsToSend;
	bcs->priority=priority;
	bcs->reliability=reliability;
	bcs->orderingChannel=orderingChannel;
	bcs->playerId=playerId;
	bcs->broadcast=broadcast;
	bcs->connectionMode=connectionMode;
	bcs->command=BufferedCommandStruct::BCS_SEND;
	bufferedCommands.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SendImmediate( char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool useCallerDataAllocation, RakNetTimeNS currentTime )
{
	unsigned *sendList;
	unsigned sendListSize;
	bool callerDataAllocationUsed;
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
	unsigned numberOfBytesUsed = BITS_TO_BYTES(numberOfBitsToSend);
	callerDataAllocationUsed=false;

	sendListSize=0;

	// 03/06/06 - If broadcast is false, use the optimized version of GetIndexFromPlayerID
	if (broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( playerId, true );
		if (remoteSystemIndex!=(unsigned)-1 &&
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
	//sendList=(unsigned *)alloca(sizeof(unsigned)*remoteSystemListSize);
		sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
	//sendList = new unsigned[remoteSystemListSize];
		sendList = new unsigned[maximumNumberOfPeers];
#endif

		// remoteSystemList in network thread
		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].isActive && remoteSystemList[ remoteSystemIndex ].playerId != playerId && remoteSystemList[ remoteSystemIndex ].playerId != UNASSIGNED_PLAYER_ID )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0)
	{
#if defined(_COMPATIBILITY_1)
		delete [] sendList;
#endif
		return false;
	}

	for (sendListIndex=0; sendListIndex < sendListSize; sendListIndex++)
	{
		if ( trackFrequencyTable )
		{
			unsigned i;
			// Store output frequency
			for (i=0 ; i < numberOfBytesUsed; i++ )
				frequencyTable[ (unsigned char)(data[i]) ]++;
			rawBytesSent += numberOfBytesUsed;
		}

		if ( outputTree )
		{
			RakNet::BitStream bitStreamCopy( numberOfBytesUsed );
			outputTree->EncodeArray( (unsigned char*) data, numberOfBytesUsed, &bitStreamCopy );
			compressedBytesSent += bitStreamCopy.GetNumberOfBytesUsed();
			remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send( (char*) bitStreamCopy.GetData(), bitStreamCopy.GetNumberOfBitsUsed(), priority, reliability, orderingChannel, true, MTUSize, currentTime );
		}
		else
		{
			// Send may split the packet and thus deallocate data.  Don't assume data is valid if we use the callerAllocationData
			bool useData = useCallerDataAllocation && callerDataAllocationUsed==false && sendListIndex+1==sendListSize;
			remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send( data, numberOfBitsToSend, priority, reliability, orderingChannel, useData==false, MTUSize, currentTime );
			if (useData)
				callerDataAllocationUsed=true;
		}

		if (reliability==RELIABLE || reliability==RELIABLE_ORDERED || reliability==RELIABLE_SEQUENCED)
			remoteSystemList[sendList[sendListIndex]].lastReliableSend=(RakNetTime)(currentTime/(RakNetTimeNS)1000);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	// Return value only meaningful if true was passed for useCallerDataAllocation.  Means the reliability layer used that data copy, so the caller should not deallocate it
	return callerDataAllocationUsed;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::HandleBufferedRPC(BufferedCommandStruct *bcs, RakNetTime time)
{
	unsigned *sendList;
	bool callerAllocationDataUsed;
	unsigned sendListSize;

	// All this code modifies bcs->data and bcs->numberOfBitsToSend in order to transform an RPC request into an actual packet for 4
	RPCIndex rpcIndex; // Index into the list of RPC calls so we know what number to encode in the packet
	char uniqueID[256], *userData; // RPC ID (the name of it) and a pointer to the data sent by the user
	int extraBuffer; // How many data bytes were allocated to hold the RPC header
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
	int dataBlockAllocationLength; // Total number of bytes to allocate for the packet
	char *writeTarget; // Used to hold either a block of allocated data or the externally allocated data

	strcpy(uniqueID, bcs->data); // Copy out the string because it is at the front of the data block and will be overwritten
	extraBuffer=2+(int)strlen(uniqueID)*2+3; // Exact code copied earlier in this file.  Keep these two in synch!
	userData=bcs->data+extraBuffer;
	dataBlockAllocationLength=BITS_TO_BYTES(bcs->numberOfBitsToSend)+extraBuffer;

	sendListSize=0;

	// 03/06/06 - If broadcast is false, use the optimized version of GetIndexFromPlayerID
	if (bcs->broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( bcs->playerId, true );
		if (remoteSystemIndex!=(unsigned)-1)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
	sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
	sendList = new unsigned[maximumNumberOfPeers];
#endif

		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].playerId != UNASSIGNED_PLAYER_ID && remoteSystemList[ remoteSystemIndex ].playerId != bcs->playerId )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0)
	{
		#if defined(_COMPATIBILITY_1)
		delete [] sendList;
		#endif

		return false;
	}

	// remoteSystemList in network thread
	for (sendListIndex=0; sendListIndex < (unsigned)sendListSize; sendListIndex++)
	{
		if (sendListIndex+1==sendListSize)
			writeTarget=bcs->data; // Write to the externally allocated buffer.  This destroys the buffer format so we do it only once for the last call
		else
			writeTarget=new char [dataBlockAllocationLength]; // Create a new buffer

		// Last send so use the buffer that was allocated externally
		RakNet::BitStream outgoingBitStream((unsigned char *) writeTarget, dataBlockAllocationLength, false );
		outgoingBitStream.ResetWritePointer(); // Let us write at the start of the data block, rather than at the end

		outgoingBitStream.Write( (unsigned char) ID_RPC );
		rpcIndex=remoteSystemList[sendList[sendListIndex]].rpcMap.GetIndexFromFunctionName(uniqueID); // Lots of trouble but we can only use remoteSystem->[whatever] in this thread so that is why this command was buffered
		if (rpcIndex!=UNDEFINED_RPC_INDEX)
		{
			// We have an RPC name to an index mapping, so write the index
			outgoingBitStream.Write(false);
			outgoingBitStream.WriteCompressed(rpcIndex);
		}
		else
		{
			// No mapping, so write the encoded RPC name
			outgoingBitStream.Write(true);
			stringCompressor->EncodeString(uniqueID, 256, &outgoingBitStream);
		}
		outgoingBitStream.Write(bcs->blockingCommand);
		outgoingBitStream.Write((bool)(bcs->command==BufferedCommandStruct::BCS_RPC_SHIFT)); // True or false to shift the timestamp
		outgoingBitStream.WriteCompressed( bcs->numberOfBitsToSend );
		if (bcs->networkID==UNASSIGNED_NETWORK_ID)
		{
			// No object ID
			outgoingBitStream.Write(false);
		}
		else
		{
			// Encode an object ID.  This will use pointer to class member RPC
			outgoingBitStream.Write(true);
			outgoingBitStream.Write(bcs->networkID);
		}


		if ( bcs->numberOfBitsToSend > 0 )
			outgoingBitStream.DoWriteBits( (const unsigned char*) userData, bcs->numberOfBitsToSend, false ); // Last param is false to write the raw data originally from another bitstream, rather than shifting from user data
		else
			outgoingBitStream.WriteCompressed( ( int ) 0 );

		callerAllocationDataUsed=SendImmediate((char*)outgoingBitStream.GetData(), outgoingBitStream.GetNumberOfBitsUsed(), bcs->priority, bcs->reliability, bcs->orderingChannel, remoteSystemList[sendList[sendListIndex]].playerId, false, true, time);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	return callerAllocationDataUsed;
}
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearBufferedCommands(void)
{
	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	while ((bcs=bufferedCommands.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

        bufferedCommands.ReadUnlock();
	}
	bufferedCommands.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearRequestedConnectionList(void)
{
	RequestedConnectionStruct *bcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	while ((bcs=requestedConnectionList.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

		requestedConnectionList.ReadUnlock();
	}
	requestedConnectionList.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
}
inline void RakPeer::AddPacketToProducer(Packet *p)
{
	Packet **packetPtr=packetSingleProducerConsumer.WriteLock();
	*packetPtr=p;
	packetSingleProducerConsumer.WriteUnlock();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace RakNet
{
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	#ifdef _MSC_VER
	#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
	#endif
	#ifdef _WIN32
	void __stdcall ProcessPortUnreachable( unsigned int binaryAddress, unsigned short port, RakPeer *rakPeer )
	#else
	void ProcessPortUnreachable( unsigned int binaryAddress, unsigned short port, RakPeer *rakPeer )
	#endif
	{
	
	}
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	#ifdef _WIN32
	void __stdcall ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer )
	#else
	void ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer )
	#endif
	{
		Packet *packet;
		PlayerID playerId;
		unsigned i;
		RakPeer::RemoteSystemStruct *remoteSystem;
		playerId.binaryAddress = binaryAddress;
		playerId.port = port;

	#if !defined(_COMPATIBILITY_1)
		if (rakPeer->IsBanned( rakPeer->PlayerIDToDottedIP( playerId ) ))
		{
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

			char c[2];
			c[0] = ID_CONNECTION_BANNED;
			c[1] = 0; // Pad, some routers apparently block 1 byte packets

			unsigned i;
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
			SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );

			return;
		}
	#endif

		// We didn't check this datagram to see if it came from a connected system or not yet.
		// Therefore, this datagram must be under 17 bits - otherwise it may be normal network traffic as the min size for a raknet send is 17 bits
		if ((unsigned char)(data)[0] == (unsigned char) ID_OPEN_CONNECTION_REPLY && length <= sizeof(unsigned char)*2)
		{
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

			// Verify that we were waiting for this
		//	bool acceptOpenConnection;
	//		int actionToTake=0;
	//		char data[MAX_OFFLINE_DATA_LENGTH];
			RakPeer::RequestedConnectionStruct *rcsFirst, *rcs;
			rcsFirst = rakPeer->requestedConnectionList.ReadLock();
			rcs=rcsFirst;
		//	acceptOpenConnection=false;
			while (rcs)
			{
				// Scan through the requested connection queue and process any elements whose playerId matches the player we just got this packet from.
				// If it is the first element in the queue, remove it from the queue.  Otherwise, set the playerId to UNASSIGNED_PLAYER_ID to cancel it out of order.
				if (rcs->playerId==playerId)
				{
					// Go ahead and process this request
				//	acceptOpenConnection=true;

					// Store the action (may be multiple actions to take at once)
			//		actionToTake|=(int)rcs->actionToTake;
					RakAssert(rcs->actionToTake==RakPeer::RequestedConnectionStruct::CONNECT);

					// You might get this when already connected because of cross-connections
					remoteSystem=rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
					if (remoteSystem==0)
						remoteSystem=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RakPeer::RemoteSystemStruct::UNVERIFIED_SENDER);

					if (remoteSystem)
					{
						RakNetTimeNS time = RakNet::GetTimeNS();
						remoteSystem->connectMode=RakPeer::RemoteSystemStruct::REQUESTED_CONNECTION;
						remoteSystem->weInitiatedTheConnection=true;

						RakNet::BitStream temp;

						temp.Write( (unsigned char) ID_CONNECTION_REQUEST );
						if ( rcs->outgoingPasswordLength > 0 )
							temp.Write( ( char* ) rcs->outgoingPassword,  rcs->outgoingPasswordLength );
						rakPeer->SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, time );
					}

					if (rcs==rcsFirst)
					{
						// Delete the head of the queue
						rakPeer->requestedConnectionList.ReadUnlock();
						rcsFirst=rakPeer->requestedConnectionList.ReadLock();
						rcs=rcsFirst;
						continue;
					}
					else
					{
						// Cancel this out of order element of the queue - we are handling it now.
						rcs->playerId=UNASSIGNED_PLAYER_ID;
					}
				}

				rcs=rakPeer->requestedConnectionList.ReadLock();
			}

			// Go back to the current head of the queue
			if (rcsFirst)
				rakPeer->requestedConnectionList.CancelReadLock(rcsFirst);

			/*
			if (acceptOpenConnection)
			{
				// You might get this when already connected because of cross-connections
				remoteSystem=rakPeer->GetRemoteSystemFromPlayerID( playerId, true );
				if (remoteSystem==0)
				{
					remoteSystem=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RakPeer::RemoteSystemStruct::UNVERIFIED_SENDER);
				}
				if (remoteSystem)
				{
					RakNetTime time = RakNet::GetTime();
					if (actionToTake & RakPeer::RequestedConnectionStruct::CONNECT)
					{
						remoteSystem->connectMode=RakPeer::RemoteSystemStruct::REQUESTED_CONNECTION;
						remoteSystem->weInitiatedTheConnection=true;

						RakNet::BitStream temp;

						temp.Write( (unsigned char) ID_CONNECTION_REQUEST );
						if ( rakPeer->outgoingPasswordLength > 0 )
							temp.Write( ( char* ) rakPeer->outgoingPassword,  rakPeer->outgoingPasswordLength );
						rakPeer->SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, time );
					}
				}
			}
			*/

			return;
		}
		// Connecting to a system we are already connected to.
		else if ((unsigned char)(data)[0] == (unsigned char) ID_CONNECTION_ATTEMPT_FAILED && length <= sizeof(unsigned char)*2)
		{
			// Remove the connection attempt from the buffered commands
			RakPeer::RequestedConnectionStruct *rcsFirst, *rcs;
			rcsFirst = rakPeer->requestedConnectionList.ReadLock();
			rcs=rcsFirst;
			bool connectionAttemptCancelled=false;
			while (rcs)
			{
				if (rcs->actionToTake==RakPeer::RequestedConnectionStruct::CONNECT && rcs->playerId==playerId)
				{
					connectionAttemptCancelled=true;
					if (rcs==rcsFirst)
					{
						rakPeer->requestedConnectionList.ReadUnlock();
						rcsFirst=rakPeer->requestedConnectionList.ReadLock();
						rcs=rcsFirst;
					}
					else
					{
						// Hole in the middle
						rcs->playerId=UNASSIGNED_PLAYER_ID;
						rcs=rakPeer->requestedConnectionList.ReadLock();
					}

					continue;
				}

				rcs=rakPeer->requestedConnectionList.ReadLock();
			}

			if (rcsFirst)
				rakPeer->requestedConnectionList.CancelReadLock(rcsFirst);

			if (connectionAttemptCancelled)
			{
				// Tell user of connection attempt failed
				packet=AllocPacket(sizeof( char ));
				packet->data[ 0 ] = ID_CONNECTION_ATTEMPT_FAILED; // Attempted a connection and couldn't
				packet->bitSize = ( sizeof( char ) * 8);
				packet->playerId = playerId;
				packet->playerIndex = 65535;
				rakPeer->AddPacketToProducer(packet);
			}
		}
		// We didn't check this datagram to see if it came from a connected system or not yet.
		// Therefore, this datagram must be under 17 bits - otherwise it may be normal network traffic as the min size for a raknet send is 17 bits
		else if ((unsigned char)(data)[0] == ID_OPEN_CONNECTION_REQUEST && length == sizeof(unsigned char)*3)
		{
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

			// If this guy is already connected and they initiated the connection, ignore the connection request
			RakPeer::RemoteSystemStruct *rss = rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
			if (rss==0 || rss->weInitiatedTheConnection==true)
			{
				// Assign new remote system
				if (rss==0)
					rss=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RakPeer::RemoteSystemStruct::UNVERIFIED_SENDER);

				unsigned char c[2];
				if (rss) // If this guy is already connected remote system will be 0
					c[0] = ID_OPEN_CONNECTION_REPLY;
				else
					c[0] = ID_NO_FREE_INCOMING_CONNECTIONS;
				c[1] = 0; // Pad, some routers apparently block 1 byte packets

				unsigned i;
				for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
					rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
				SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );

				return;
			}
			else if (rss!=0)
			{
				// If this is an existing connection, and they are already fully connected (not in progress), reply with connection attempt failed
				if (rss->connectMode==RakPeer::RemoteSystemStruct::CONNECTED ||
					rss->connectMode==RakPeer::RemoteSystemStruct::DISCONNECT_ASAP ||
					rss->connectMode==RakPeer::RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY)
				{
					char c[2];
					c[0] = ID_CONNECTION_ATTEMPT_FAILED;
					c[1] = 0; // Pad, some routers apparently block 1 byte packets

					unsigned i;
					for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
						rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
					SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );
				}
			}

		}

		// See if this datagram came from a connected system
		remoteSystem = rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
		if ( remoteSystem )
		{
			if (remoteSystem->connectMode==RakPeer::RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET &&
	#if RAKNET_LEGACY
					(length%8)==0
	#else
					(length%16)==0
	#endif
					)
				remoteSystem->reliabilityLayer.SetEncryptionKey( remoteSystem->AESKey );

			// Handle regular incoming data
			// HandleSocketReceiveFromConnectedPlayer is only safe to be called from the same thread as Update, which is this thread
			if ( remoteSystem->reliabilityLayer.HandleSocketReceiveFromConnectedPlayer( data, length, playerId, rakPeer->messageHandlerList, rakPeer->MTUSize ) == false )
			{
				// These kinds of packets may have been duplicated and incorrectly determined to be
				// cheat packets.  Anything else really is a cheat packet
				if ( !(
				( (unsigned char)data[0] == ID_OPEN_CONNECTION_REQUEST && length <= 3 ) ||
				( (unsigned char)data[0] == ID_OPEN_CONNECTION_REPLY && length <= 2 ) ||
				( (unsigned char)data[0] == ID_CONNECTION_ATTEMPT_FAILED && length <= 2 ) ||
				( ((unsigned char)data[0] == ID_PING_OPEN_CONNECTIONS || (unsigned char)data[0] == ID_PING || (unsigned char)data[0] == ID_PONG) && length >= sizeof(unsigned char)+sizeof(RakNetTime) ) ||
				( (unsigned char)data[0] == ID_ADVERTISE_SYSTEM && length<MAX_OFFLINE_DATA_LENGTH )
				) )
				{
					// Unknown message.  Could be caused by old out of order stuff from unconnected or no longer connected systems, etc.
					packet=AllocPacket(1);
					packet->data[ 0 ] = ID_MODIFIED_PACKET;
					packet->bitSize = sizeof( char ) * 8;
					packet->playerId = playerId;
					packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
					rakPeer->AddPacketToProducer(packet);
				}
			}
		}
		else
		{
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

			if (length > 512)
			{
	#if !defined(_COMPATIBILITY_1)
				// Flood attack?  Unknown systems should never send more than a small amount of data. Do a short ban
				rakPeer->AddToBanList(rakPeer->PlayerIDToDottedIP(playerId), 10000);
	#endif
				return;
			}

			// These are all messages from unconnected systems.  Messages here can be any size, but are never processed from connected systems.
			if ( ( (unsigned char) data[ 0 ] == ID_PING_OPEN_CONNECTIONS
				|| (unsigned char)(data)[0] == ID_PING)	&& length == sizeof(unsigned char)+sizeof(RakNetTime) )
			{
				if ( (unsigned char)(data)[0] == ID_PING ||
					rakPeer->AllowIncomingConnections() ) // Open connections with players
				{
	#if !defined(_COMPATIBILITY_1)
					RakNet::BitStream inBitStream( (unsigned char *) data, length, false );
					inBitStream.IgnoreBits(8);
					RakNetTime sendPingTime;
					inBitStream.Read(sendPingTime);

					RakNet::BitStream outBitStream;
					outBitStream.Write((unsigned char)ID_PONG); // Should be named ID_UNCONNECTED_PONG eventually
					outBitStream.Write(sendPingTime);
					//tempBitStream.Write( data, UnconnectedPingStruct_Size );
					rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Lock();
					// They are connected, so append offline ping data
					outBitStream.Write( (char*)rakPeer->offlinePingResponse.GetData(), rakPeer->offlinePingResponse.GetNumberOfBytesUsed() );
					rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Unlock();
					//SocketLayer::Instance()->SendTo( connectionSocket, ( char* ) outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), playerId.binaryAddress, playerId.port );

					unsigned i;
					for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
						rakPeer->messageHandlerList[i]->OnDirectSocketSend((const char*)outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), playerId);

					SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (const char*)outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), (char*)rakPeer->PlayerIDToDottedIP(playerId) , playerId.port );
	#endif
				}
			}
			// UNCONNECTED MESSAGE Pong with no data.  TODO - Problem - this matches a reliable send of other random data.
			else if ((unsigned char) data[ 0 ] == ID_PONG && length >= sizeof(unsigned char)+sizeof(RakNetTime) && length < sizeof(unsigned char)+sizeof(RakNetTime)+MAX_OFFLINE_DATA_LENGTH)
			{
				packet=AllocPacket(length);
				memcpy(packet->data, data, length);
				packet->bitSize = length * 8;
				packet->playerId = playerId;
				packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
				rakPeer->AddPacketToProducer(packet);
			}
			else if ((unsigned char) data[ 0 ] == ID_ADVERTISE_SYSTEM && length >= 2 && length < MAX_OFFLINE_DATA_LENGTH+2)
			{
				packet=AllocPacket(length);
				memcpy(packet->data, data, length);
				packet->bitSize = length * 8;
				packet->playerId = playerId;
				packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
				rakPeer->AddPacketToProducer(packet);
			}
		}
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool RakPeer::RunUpdateCycle( void )
	{
		RakPeer::RemoteSystemStruct * remoteSystem;
		unsigned remoteSystemIndex;
		Packet *packet;
		RakNetTime ping, lastPing;
		// int currentSentBytes,currentReceivedBytes;
	//	unsigned numberOfBytesUsed;
		unsigned numberOfBitsUsed;
		//PlayerID authoritativeClientPlayerId;
		int bitSize, byteSize;
		unsigned char *data;
		int errorCode;
		int gotData;
		RakNetTimeNS timeNS;
		RakNetTime timeMS;
		PlayerID playerId;
		BufferedCommandStruct *bcs;
		bool callerDataAllocationUsed;
		RakNetStatisticsStruct *rnss;

		do
		{
			// Read a packet
			gotData = SocketLayer::Instance()->RecvFrom( connectionSocket, this, &errorCode );

			if ( gotData == SOCKET_ERROR )
			{
	#ifdef _WIN32
				if ( errorCode == WSAECONNRESET )
				{
					gotData=false;
					// 11/14/05 - RecvFrom now calls HandlePortUnreachable rather than PushPortRefused
					//PushPortRefused( UNASSIGNED_PLAYER_ID );
					//closesocket(peer->connectionSocket);

					//peer->connectionSocket = SocketLayer::Instance()->CreateBoundSocket(peer->myPlayerId.port, true);
				}
				else
					if ( errorCode != 0 && endThreads == false )
					{
	#ifdef _DO_PRINTF
						printf( "Server RecvFrom critical failure!\n" );
	#endif
						// Some kind of critical error
						// peer->isRecvfromThreadActive=false;
						endThreads = true;
						Disconnect( 0, 0 );
						return false;
					}

	#else
				if ( errorCode == -1 )
				{
					// isRecvfromThreadActive=false;
					endThreads = true;
					Disconnect( 0 );
					return false;
				}
	#endif
			}

			if ( endThreads )
				return false;
		}
		while ( gotData>0 ); // Read until there is nothing left

		timeNS=0;
		timeMS=0;

		// Process all the deferred user thread Send and connect calls
		while ((bcs=bufferedCommands.ReadLock())!=0)
		{
			if (bcs->command==BufferedCommandStruct::BCS_SEND)
			{
				// GetTime is a very slow call so do it once and as late as possible
				if (timeNS==0)
					timeNS = RakNet::GetTimeNS();

				callerDataAllocationUsed=SendImmediate((char*)bcs->data, bcs->numberOfBitsToSend, bcs->priority, bcs->reliability, bcs->orderingChannel, bcs->playerId, bcs->broadcast, true, timeNS);
				if ( callerDataAllocationUsed==false )
					delete bcs->data;

				// Set the new connection state AFTER we call sendImmediate in case we are setting it to a disconnection state, which does not allow further sends
				if (bcs->connectionMode!=RemoteSystemStruct::NO_ACTION && bcs->playerId!=UNASSIGNED_PLAYER_ID)
				{
					remoteSystem=GetRemoteSystemFromPlayerID( bcs->playerId, true, true );
				//	if (remoteSystem==0)
				//		remoteSystem=AssignSystemAddressToRemoteSystemList(bcs->systemAddress, bcs->connectionMode);
					if (remoteSystem)
						remoteSystem->connectMode=bcs->connectionMode;
				}
			}
			else
			{

				RakAssert(bcs->command==BufferedCommandStruct::BCS_CLOSE_CONNECTION);

				CloseConnectionInternal(bcs->playerId, false, true, bcs->orderingChannel);
			}

	#ifdef _DEBUG
			bcs->data=0;
	#endif

			bufferedCommands.ReadUnlock();
		}

		// Process connection attempts
		RequestedConnectionStruct *rcsFirst, *rcs;
		bool condition1, condition2;
		rcsFirst = requestedConnectionList.ReadLock();
		rcs=rcsFirst;
		while (rcs)
		{
			if (timeNS==0)
			{
				timeNS = RakNet::GetTimeNS();
				timeMS = (RakNetTime)(timeNS/(RakNetTimeNS)1000);
			}

			if (rcs->nextRequestTime < timeMS)
			{
				condition1=rcs->requestsMade==6;
				condition2=(bool)((rcs->playerId==UNASSIGNED_PLAYER_ID)==1);
				// If too many requests made or a hole then remove this if possible, otherwise invalidate it
				if (condition1 || condition2)
				{
					if (rcs->data)
					{
						delete [] rcs->data;
						rcs->data=0;
					}

					if (condition1 && !condition2 && rcs->actionToTake==RequestedConnectionStruct::CONNECT)
					{
						// Tell user of connection attempt failed
						packet=AllocPacket(sizeof( char ));
						packet->data[ 0 ] = ID_CONNECTION_ATTEMPT_FAILED; // Attempted a connection and couldn't
						packet->bitSize = ( sizeof( char ) * 8);
						packet->playerId = rcs->playerId;
						packet->playerIndex = 65535;
						AddPacketToProducer(packet);
					}

					// Remove this if possible
					if (rcs==rcsFirst)
					{
						requestedConnectionList.ReadUnlock();
						rcsFirst = requestedConnectionList.ReadLock();
						rcs=rcsFirst;
					}
					else
					{
						// Hole in the middle
						rcs->playerId=UNASSIGNED_PLAYER_ID;
						rcs=requestedConnectionList.ReadLock();
					}

					continue;
				}

				rcs->requestsMade++;
				rcs->nextRequestTime=timeMS+1000;
				char c[3];
				c[0] = ID_OPEN_CONNECTION_REQUEST;
				c[1] = 0; // Pad - apparently some routers block 1 byte packets

				unsigned i;
				for (i=0; i < messageHandlerList.Size(); i++)
					messageHandlerList[i]->OnDirectSocketSend((char*)&c, 24, rcs->playerId);
				SocketLayer::Instance()->SendTo( connectionSocket, (char*)&c, 3, rcs->playerId.binaryAddress, rcs->playerId.port );
			}

			rcs=requestedConnectionList.ReadLock();
		}

		if (rcsFirst)
			requestedConnectionList.CancelReadLock(rcsFirst);

	 
		// remoteSystemList in network thread
		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; ++remoteSystemIndex )
		//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; ++remoteSystemIndex )
		{
			// I'm using playerId from remoteSystemList but am not locking it because this loop is called very frequently and it doesn't
			// matter if we miss or do an extra update.  The reliability layers themselves never care which player they are associated with
			//playerId = remoteSystemList[ remoteSystemIndex ].playerId;
			// Allow the playerID for this remote system list to change.  We don't care if it changes now.
		//	remoteSystemList[ remoteSystemIndex ].allowPlayerIdAssigment=true;
			if ( remoteSystemList[ remoteSystemIndex ].isActive )
			{
				playerId = remoteSystemList[ remoteSystemIndex ].playerId;
				RakAssert(playerId!=UNASSIGNED_PLAYER_ID);

				// Found an active remote system
				remoteSystem = remoteSystemList + remoteSystemIndex;
				// Update is only safe to call from the same thread that calls HandleSocketReceiveFromConnectedPlayer,
				// which is this thread

				if (timeNS==0)
				{
					timeNS = RakNet::GetTimeNS();
					timeMS = (RakNetTime)(timeNS/(RakNetTimeNS)1000);
					//printf("timeNS = %I64i timeMS=%i\n", timeNS, timeMS);
				}


				if (timeMS > remoteSystem->lastReliableSend && timeMS-remoteSystem->lastReliableSend > 5000 && remoteSystem->connectMode==RemoteSystemStruct::CONNECTED)
				{
					// If no reliable packets are waiting for an ack, do a one byte reliable send so that disconnections are noticed
					rnss=remoteSystem->reliabilityLayer.GetStatistics();
					if (rnss->messagesOnResendQueue==0)
					{
						unsigned char keepAlive=ID_DETECT_LOST_CONNECTIONS;
						SendImmediate((char*)&keepAlive,8,LOW_PRIORITY, RELIABLE, 0, remoteSystem->playerId, false, false, timeNS);
						remoteSystem->lastReliableSend=timeMS+remoteSystem->reliabilityLayer.GetTimeoutTime();
					}
				}

				remoteSystem->reliabilityLayer.Update( connectionSocket, playerId, MTUSize, timeNS, messageHandlerList ); // playerId only used for the internet simulator test

				// Check for failure conditions
				if ( remoteSystem->reliabilityLayer.IsDeadConnection() ||
					((remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY) && remoteSystem->reliabilityLayer.IsDataWaiting()==false) ||
					(remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK && remoteSystem->reliabilityLayer.AreAcksWaiting()==false) ||
					((
					(remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION ||
					remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST ||
					remoteSystem->connectMode==RemoteSystemStruct::UNVERIFIED_SENDER ||
					remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET)
					&& timeMS > remoteSystem->connectionTime && timeMS - remoteSystem->connectionTime > 10000))
					)
				{
				//	printf("timeMS=%i remoteSystem->connectionTime=%i\n", timeMS, remoteSystem->connectionTime );

					// Failed.  Inform the user?
					if (remoteSystem->connectMode==RemoteSystemStruct::CONNECTED || remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION
						|| remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
					{
						// Inform the user of the connection failure.
					//	unsigned staticDataBytes;

					//	staticDataBytes=remoteSystem->staticData.GetNumberOfBytesUsed();
					//	packet=AllocPacket(sizeof( char ) + staticDataBytes);
						packet=AllocPacket(sizeof( char ) );
						if (remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION)
							packet->data[ 0 ] = ID_CONNECTION_ATTEMPT_FAILED; // Attempted a connection and couldn't
						else if (remoteSystem->connectMode==RemoteSystemStruct::CONNECTED)
							packet->data[ 0 ] = ID_CONNECTION_LOST; // DeadConnection
						else
							packet->data[ 0 ] = ID_DISCONNECTION_NOTIFICATION; // DeadConnection

						//if (staticDataBytes)
						//	memcpy( packet->data + sizeof( char ), remoteSystem->staticData.GetData(), staticDataBytes );
						packet->bitSize = ( sizeof( char ) ) * 8;
						//packet->bitSize = ( sizeof( char ) + staticDataBytes ) * 8;
						packet->playerId = playerId;
						packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;

						AddPacketToProducer(packet);
					}
					// else connection shutting down, don't bother telling the user

	#ifdef _DO_PRINTF
					printf("Connection dropped for player %i:%i\n", playerId.binaryAddress, playerId.port);
	#endif
					CloseConnectionInternal( playerId, false, true, 0 );
					continue;
				}

				// Did the reliability layer detect a modified packet?
				if ( remoteSystem->reliabilityLayer.IsCheater() )
				{
					packet=AllocPacket(sizeof(char));
					packet->bitSize=8;
					packet->data[ 0 ] = (unsigned char) ID_MODIFIED_PACKET;
					packet->playerId = playerId;
					packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
					AddPacketToProducer(packet);
					continue;
				}

				// Ping this guy if it is time to do so
				if ( remoteSystem->connectMode==RemoteSystemStruct::CONNECTED && timeMS > remoteSystem->nextPingTime && ( occasionalPing || remoteSystem->lowestPing == (unsigned short)-1 ) )
				{
					remoteSystem->nextPingTime = timeMS + 5000;
					PingInternal( playerId, true );
				}

				// Find whoever has the lowest player ID
				//if (playerId < authoritativeClientPlayerId)
				// authoritativeClientPlayerId=playerId;

				// Does the reliability layer have any packets waiting for us?
				// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
				bitSize = remoteSystem->reliabilityLayer.Receive( &data );

				while ( bitSize > 0 )
				{
					// These types are for internal use and should never arrive from a network packet
					if (data[0]==ID_CONNECTION_ATTEMPT_FAILED || data[0]==ID_MODIFIED_PACKET)
					{
						RakAssert(0);
						continue;
					}

					// Put the input through compression if necessary
					if ( inputTree )
					{
						RakNet::BitStream dataBitStream( MAXIMUM_MTU_SIZE );
						// Since we are decompressing input, we need to copy to a bitstream, decompress, then copy back to a probably
						// larger data block.  It's slow, but the user should have known that anyway
						dataBitStream.Reset();
						dataBitStream.WriteAlignedBytes( ( unsigned char* ) data, BITS_TO_BYTES( bitSize ) );
						rawBytesReceived += dataBitStream.GetNumberOfBytesUsed();

	//					numberOfBytesUsed = dataBitStream.GetNumberOfBytesUsed();
						numberOfBitsUsed = dataBitStream.GetNumberOfBitsUsed();
						//rawBytesReceived += numberOfBytesUsed;
						// Decompress the input data.

						if (numberOfBitsUsed>0)
						{
							unsigned char *dataCopy = new unsigned char[ dataBitStream.GetNumberOfBytesUsed() ];
							memcpy( dataCopy, dataBitStream.GetData(), dataBitStream.GetNumberOfBytesUsed() );
							dataBitStream.Reset();
							inputTree->DecodeArray( dataCopy, numberOfBitsUsed, &dataBitStream );
							compressedBytesReceived += dataBitStream.GetNumberOfBytesUsed();
							delete [] dataCopy;

							byteSize = dataBitStream.GetNumberOfBytesUsed();

							if ( byteSize > BITS_TO_BYTES( bitSize ) )   // Probably the case - otherwise why decompress?
							{
								delete [] data;
								data = new unsigned char [ byteSize ];
							}
							memcpy( data, dataBitStream.GetData(), byteSize );
						}
						else
							byteSize=0;
					}
					else
						// Fast and easy - just use the data that was returned
						byteSize = BITS_TO_BYTES( bitSize );

					// For unknown senders we only accept a few specific packets
					if (remoteSystem->connectMode==RemoteSystemStruct::UNVERIFIED_SENDER)
					{
						if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST )
						{
							ParseConnectionRequestPacket(remoteSystem, playerId, (const char*)data, byteSize);
							delete [] data;
						}
						else
						{
							CloseConnectionInternal( playerId, false, true, 0 );
	#ifdef _DO_PRINTF
							printf("Temporarily banning %i:%i for sending nonsense data\n", playerId.binaryAddress, playerId.port);
	#endif

	#if !defined(_COMPATIBILITY_1)
							AddToBanList(PlayerIDToDottedIP(playerId), remoteSystem->reliabilityLayer.GetTimeoutTime());
	#endif
							delete [] data;
						}
					}
					else
					{
						// However, if we are connected we still take a connection request in case both systems are trying to connect to each other
						// at the same time
						if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST )
						{
							// 04/27/06 This is wrong.  With cross connections, we can both have initiated the connection are in state REQUESTED_CONNECTION
							// 04/28/06 Downgrading connections from connected will close the connection due to security at ((remoteSystem->connectMode!=RemoteSystemStruct::CONNECTED && time > remoteSystem->connectionTime && time - remoteSystem->connectionTime > 10000))
							if (remoteSystem->connectMode!=RemoteSystemStruct::CONNECTED)
								ParseConnectionRequestPacket(remoteSystem, playerId, (const char*)data, byteSize);
							delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_NEW_INCOMING_CONNECTION && byteSize == sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short) )
						{

							// This assert can be ignored since it could hit from duplicate packets.
							// It's just here for internal testing since it should only happen rarely and will mostly be from bugs
	//						RakAssert(remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST);

							if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST ||
								remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET ||
								playerId==myPlayerId) // local system connect
							{
								remoteSystem->connectMode=RemoteSystemStruct::CONNECTED;
								PingInternal( playerId, true );
								SendStaticDataInternal( playerId, true );

								RakNet::BitStream inBitStream((unsigned char *) data, byteSize, false);
								PlayerID bsPlayerId;

								inBitStream.IgnoreBits(8);
								inBitStream.Read(bsPlayerId.binaryAddress);
								inBitStream.Read(bsPlayerId.port);

								// Overwrite the data in the packet
								//					NewIncomingConnectionStruct newIncomingConnectionStruct;
								//					RakNet::BitStream nICS_BS( data, NewIncomingConnectionStruct_Size, false );
								//					newIncomingConnectionStruct.Deserialize( nICS_BS );
								remoteSystem->myExternalPlayerId = bsPlayerId;

								// Send this info down to the game

								packet=AllocPacket(byteSize, data);
								packet->bitSize = bitSize;
								packet->playerId = playerId;
								packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
								AddPacketToProducer(packet);
							}
							else
								delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_CONNECTED_PONG && byteSize == sizeof(unsigned char)+sizeof(RakNetTime)*2 )
						{
							RakNetTime sendPingTime, sendPongTime;

							// Copy into the ping times array the current time - the value returned
							// First extract the sent ping
							RakNet::BitStream inBitStream( (unsigned char *) data, byteSize, false );
							//PingStruct ps;
							//ps.Deserialize(psBS);
							inBitStream.IgnoreBits(8);
							inBitStream.Read(sendPingTime);
							inBitStream.Read(sendPongTime);

							timeNS = RakNet::GetTimeNS(); // Update the time value to be accurate
							timeMS = (RakNetTime)(timeNS/(RakNetTimeNS)1000);
							if (timeMS > sendPingTime)
								ping = timeMS - sendPingTime;
							else
								ping=0;
							lastPing = remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].pingTime;

							// Ignore super high spikes in the average
							if ( lastPing <= 0 || ( ( ping < ( lastPing * 3 ) ) && ping < 1200 ) )
							{
								remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].pingTime = ( unsigned short ) ping;
								// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
								remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].clockDifferential = sendPongTime - ( timeMS + sendPingTime ) / 2;

								if ( remoteSystem->lowestPing == (unsigned short)-1 || remoteSystem->lowestPing > (int) ping )
									remoteSystem->lowestPing = (unsigned short) ping;

								// Most packets should arrive by the ping time.
								RakAssert(ping < 10000); // Sanity check - could hit due to negative pings causing the var to overflow
								remoteSystem->reliabilityLayer.SetPing( (unsigned short) ping );

								if ( ++( remoteSystem->pingAndClockDifferentialWriteIndex ) == PING_TIMES_ARRAY_SIZE )
									remoteSystem->pingAndClockDifferentialWriteIndex = 0;
							}

							delete [] data;
						}
						else if ( (unsigned char)data[0] == ID_INTERNAL_PING && byteSize == sizeof(unsigned char)+sizeof(RakNetTime) )
						{
							RakNet::BitStream inBitStream( (unsigned char *) data, byteSize, false );
 							inBitStream.IgnoreBits(8);
							RakNetTime sendPingTime;
							inBitStream.Read(sendPingTime);

							RakNet::BitStream outBitStream;
							outBitStream.Write((unsigned char)ID_CONNECTED_PONG);
							outBitStream.Write(sendPingTime);
							timeMS = RakNet::GetTime();
							timeNS = RakNet::GetTimeNS();
							outBitStream.Write(timeMS);
							SendImmediate( (char*)outBitStream.GetData(), outBitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, UNRELIABLE, 0, playerId, false, false, timeMS );

							delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_DISCONNECTION_NOTIFICATION )
						{
							// We shouldn't close the connection immediately because we need to ack the ID_DISCONNECTION_NOTIFICATION
							remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ON_NO_ACK;
							delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_RPC_MAPPING )
						{
							RakNet::BitStream inBitStream( (unsigned char *) data, byteSize, false );
							RPCIndex index;
							inBitStream.IgnoreBits(8);
	#if RPCID_STRING
							char output[256];
							stringCompressor->DecodeString(output, 255, &inBitStream);
	#endif
							inBitStream.ReadCompressed(index);
	#if !RPCID_STRING
							RPCID output;
							output = (RPCID)index;
	#endif
							remoteSystem->rpcMap.AddIdentifierAtIndex((RPCID)output, index);
							delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_REQUEST_STATIC_DATA )
						{
							SendStaticDataInternal( playerId, true );
							delete [] data;
						}
						else if ( (unsigned char) data[ 0 ] == ID_RECEIVED_STATIC_DATA )
						{
							remoteSystem->staticData.Reset();
							remoteSystem->staticData.Write( ( char* ) data + sizeof(unsigned char), byteSize - 1 );

							// Inform game server code that we got static data
							packet=AllocPacket(byteSize, data);
							packet->bitSize = bitSize;
							packet->playerId = playerId;
							packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
							AddPacketToProducer(packet);
						}
	#if !defined(_COMPATIBILITY_1)
						else if ( (unsigned char)(data)[0] == ID_SECURED_CONNECTION_RESPONSE &&
							byteSize == 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20 )
						{
							SecuredConnectionConfirmation( remoteSystem, (char*)data );
							delete [] data;
						}
						else if ( (unsigned char)(data)[0] == ID_SECURED_CONNECTION_CONFIRMATION &&
							byteSize == 1 + 20 + sizeof( RSA_BIT_SIZE ) )
						{
							CSHA1 sha1;
							bool confirmedHash, newRandNumber;

							confirmedHash = false;

							// Hash the SYN-Cookie
							// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
							sha1.Reset();
							sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
							sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
							sha1.Update( ( unsigned char* ) & ( newRandomNumber ), 20 );
							sha1.Final();

							newRandNumber = false;

							// Confirm if
							//syn-cookie ?= HASH(source ip address + source port + last random number)
							//syn-cookie ?= HASH(source ip address + source port + current random number)
							if ( memcmp( sha1.GetHash(), data + 1, 20 ) == 0 )
							{
								confirmedHash = true;
							}
							else
							{
								sha1.Reset();
								sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
								sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
								sha1.Update( ( unsigned char* ) & ( oldRandomNumber ), 20 );
								sha1.Final();

								if ( memcmp( sha1.GetHash(), data + 1, 20 ) == 0 )
									confirmedHash = true;
							}
							if ( confirmedHash )
							{
								int i;
								unsigned char AESKey[ 16 ];
								RSA_BIT_SIZE message, encryptedMessage;

								// On connection accept, AES key is c2s RSA_Decrypt(random number) XOR s2c syn-cookie
								// Get the random number first
								#ifdef HOST_ENDIAN_IS_BIG
									BSWAPCPY( (unsigned char *) encryptedMessage, (unsigned char *)(data + 1 + 20), sizeof( RSA_BIT_SIZE ) );
								#else
									memcpy( encryptedMessage, data + 1 + 20, sizeof( RSA_BIT_SIZE ) );
								#endif
								rsacrypt.decrypt( encryptedMessage, message );
								#ifdef HOST_ENDIAN_IS_BIG
									BSWAPSELF( (unsigned char *) message, sizeof( RSA_BIT_SIZE ) );
								#endif

								// Save the AES key
								for ( i = 0; i < 16; i++ )
									AESKey[ i ] = data[ 1 + i ] ^ ( ( unsigned char* ) ( message ) ) [ i ];

								// Connect this player assuming we have open slots
								OnConnectionRequest( remoteSystem, AESKey, true );
							}
							delete [] data;
						}
	#endif // #if !defined(_COMPATIBILITY_1)
						else if ( (unsigned char)(data)[0] == ID_DETECT_LOST_CONNECTIONS && byteSize == sizeof(unsigned char) )
						{
							// Do nothing
							delete [] data;
						}
						else if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST_ACCEPTED && byteSize == sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short)+sizeof(PlayerIndex)+sizeof(unsigned int))
						{
							// Make sure this connection accept is from someone we wanted to connect to
							bool allowConnection, alreadyConnected;

							if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST || remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION || allowConnectionResponseIPMigration)
								allowConnection=true;
							else
								allowConnection=false;
							if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST)
								alreadyConnected=true;
							else
								alreadyConnected=false;

							if ( allowConnection )
							{
								PlayerID externalID;
								PlayerIndex playerIndex;
								unsigned int token;

								RakNet::BitStream inBitStream((unsigned char *) data, byteSize, false);
								inBitStream.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
							//	inBitStream.Read(remotePort);
								inBitStream.Read(externalID.binaryAddress);
								inBitStream.Read(externalID.port);
								inBitStream.Read(playerIndex);
								inBitStream.Read(token);
								SAMPRakNet::SetToken(token);

								// Find a free remote system struct to use
								//						RakNet::BitStream casBitS(data, byteSize, false);
								//						ConnectionAcceptStruct cas;
								//						cas.Deserialize(casBitS);
							//	playerId.port = remotePort;

								// The remote system told us our external IP, so save it
								remoteSystem->myExternalPlayerId = externalID;
								remoteSystem->connectMode=RemoteSystemStruct::CONNECTED;


								if (alreadyConnected==false)
								{
	#ifdef __USE_IO_COMPLETION_PORTS
									bool b;
									// Create a new nonblocking socket
									remoteSystem->reliabilityLayer.SetSocket( SocketLayer::Instance()->CreateBoundSocket( myPlayerId.port, false ) );

									SocketLayer::Instance()->Connect( remoteSystem->reliabilityLayer.GetSocket(), playerId.binaryAddress, playerId.port );
									// Associate our new socket with a completion port and do the first read
									b = SocketLayer::Instance()->AssociateSocketWithCompletionPortAndRead( remoteSystem->reliabilityLayer.GetSocket(), playerId.binaryAddress, playerId.port, rakPeer );
									//client->//reliabilityLayerMutex.Unlock();

									if ( b == false )   // Some damn completion port error... windows is so unreliable
									{
	#ifdef _DO_PRINTF
										printf( "RakClient - AssociateSocketWithCompletionPortAndRead failed" );
	#endif
										return ;
									}
	#endif

									// Use the stored encryption key
									if (remoteSystem->setAESKey)
										remoteSystem->reliabilityLayer.SetEncryptionKey( remoteSystem->AESKey );
									else
										remoteSystem->reliabilityLayer.SetEncryptionKey( 0 );
								}

								// Send the connection request complete to the game
								packet=AllocPacket(byteSize, data);
								packet->bitSize = byteSize * 8;
								packet->playerId = playerId;
								packet->playerIndex = ( PlayerIndex ) GetIndexFromPlayerID( playerId, true );
								AddPacketToProducer(packet);

								RakNet::BitStream outBitStream(sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short));
								outBitStream.Write((unsigned char)ID_NEW_INCOMING_CONNECTION);
								outBitStream.Write(playerId.binaryAddress);
								outBitStream.Write(playerId.port);
								// We turned on encryption with SetEncryptionKey.  This pads packets to up to 16 bytes.
								// As soon as a 16 byte packet arrives on the remote system, we will turn on AES.  This works because all encrypted packets are multiples of 16 and the
								// packets I happen to be sending before this are less than 16 bytes.  Otherwise there is no way to know if a packet that arrived is
								// encrypted or not so the other side won't know to turn on encryption or not.
								SendImmediate( (char*)outBitStream.GetData(), outBitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime() );

								if (alreadyConnected==false)
								{
									PingInternal( playerId, true );
									SendStaticDataInternal( playerId, true );
								}
							}
							else
							{
								// Tell the remote system the connection failed
								NotifyAndFlagForDisconnect(playerId, true, 0);
	#ifdef _DO_PRINTF
								printf( "Error: Got a connection accept when we didn't request the connection.\n" );
	#endif
								delete [] data;
							}
						}
						else
						{
							if (data[0]>=(unsigned char)ID_RPC)
							{
								packet=AllocPacket(byteSize, data);
								packet->bitSize = bitSize;
								packet->playerId = playerId;
								packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
								AddPacketToProducer(packet);					
							}
							//else
								// Some internal type got returned to the user?
								//RakAssert(0);
						}
					}

					// Does the reliability layer have any more packets waiting for us?
					// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
					bitSize = remoteSystem->reliabilityLayer.Receive( &data );
				}
			}
		}

		return true;
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	#ifdef _WIN32
	unsigned __stdcall UpdateNetworkLoop( LPVOID arguments )
	#else
	void* UpdateNetworkLoop( void* arguments )
	#endif
	{
		RakPeer * rakPeer = ( RakPeer * ) arguments;
		// RakNetTime time;

	#ifdef __USE_IO_COMPLETION_PORTS

		AsynchronousFileIO::Instance()->IncreaseUserCount();
	#endif

		// 11/15/05 - this is slower than Sleep()
	#ifdef _WIN32
	#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
		// Lets see if these timers give better performance than Sleep
		HANDLE timerHandle;
		LARGE_INTEGER dueTime;

		if ( rakPeer->threadSleepTimer <= 0 )
			rakPeer->threadSleepTimer = 1;

		// 2nd parameter of false means synchronization timer instead of manual-reset timer
		timerHandle = CreateWaitableTimer( NULL, FALSE, 0 );

		RakAssert( timerHandle );

		dueTime.QuadPart = -10000 * rakPeer->threadSleepTimer; // 10000 is 1 ms?

		BOOL success = SetWaitableTimer( timerHandle, &dueTime, rakPeer->threadSleepTimer, NULL, NULL, FALSE );

		RakAssert( success );

	#endif
	#endif

	#ifdef _RAKNET_THREADSAFE
		#pragma message("-- RakNet: _RAKNET_THREADSAFE defined.  Safe to use multiple threads on the same instance of RakPeer (Slow!). --")
	#else
		#pragma message("-- RakNet: _RAKNET_THREADSAFE not defined.  Do NOT use multiple threads on the same instance of RakPeer (Fast!). --")
	#endif

		rakPeer->isMainLoopThreadActive = true;

		while ( rakPeer->endThreads == false )
		{
			rakPeer->RunUpdateCycle();
			if (rakPeer->threadSleepTimer>=0)
			{
	#if defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
				if (rakPeer->threadSleepTimer>0)
					WSAWaitForMultipleEvents(1,&rakPeer->recvEvent,TRUE,rakPeer->threadSleepTimer,FALSE);
				else
					RakSleep(0);
	#else // _WIN32
					RakSleep( rakPeer->threadSleepTimer );
	#endif
			}
		}


	#ifdef __USE_IO_COMPLETION_PORTS
		AsynchronousFileIO::Instance()->DecreaseUserCount();
	#endif

		rakPeer->isMainLoopThreadActive = false;

		return 0;
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
