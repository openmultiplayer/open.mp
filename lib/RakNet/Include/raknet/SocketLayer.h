/// \file
/// \brief \b [Internal] Encapsulates Berkely sockets
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


#ifndef __SOCKET_LAYER_H
#define __SOCKET_LAYER_H

#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h"
#elif defined(_WIN32)
// IP_DONTFRAGMENT is different between winsock 1 and winsock 2.  Therefore, Winsock2.h must be linked againt Ws2_32.lib
// winsock.h must be linked against WSock32.lib.  If these two are mixed up the flag won't work correctly
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> 
/// Unix/Linux uses ints for sockets
typedef int SOCKET;
#define INVALID_SOCKET -1 
#define SOCKET_ERROR -1
#endif
#include "ClientContextStruct.h"

namespace RakNet
{
	class RakPeer;

	// A platform independent implementation of Berkeley sockets, with settings used by RakNet
	class SocketLayer
	{

	public:
		
		/// Default Constructor
		SocketLayer();
		
		/// Destructor	
		~SocketLayer();
		
		// Get the singleton instance of the Socket Layer.
		/// \return unique instance 
		static inline SocketLayer* Instance()
		{
			if (_instance == nullptr)
				_instance = new SocketLayer();
			return _instance;
		}
		static inline void Destroy()
		{
			if (_instance != nullptr)
			{
				delete _instance;
				_instance = nullptr;
			}
		}
		
		// Connect a socket to a remote host.
		/// \param[in] writeSocket The local socket.
		/// \param[in] binaryAddress The address of the remote host.
		/// \param[in] port the remote port.
		/// \return A new socket used for communication.
		SOCKET Connect( SOCKET writeSocket, unsigned int binaryAddress, unsigned short port );
		
		// Creates a bound socket to listen for incoming connections on the specified port
		/// \param[in] port the port number 
		/// \param[in] blockingSocket 
		/// \return A new socket used for accepting clients 
		SOCKET CreateBoundSocket( unsigned short port, bool blockingSocket, const char *forceHostAddress );

		#if !defined(_COMPATIBILITY_1)
		const char* DomainNameToIP( const char *domainName );
		#endif
		
		#ifdef __USE_IO_COMPLETION_PORTS
		void AssociateSocketWithCompletionPort( SOCKET socket, ClientContextStruct* completionKey );
		#endif
		
		/// Start an asynchronous read using the specified socket.  The callback will use the specified PlayerID (associated with this socket) and call either the client or the server callback (one or
		/// the other should be 0)
		/// \note Was used for depreciated IO completion ports.	
		bool AssociateSocketWithCompletionPortAndRead( SOCKET readSocket, unsigned int binaryAddress, unsigned short port, RakPeer* rakPeer );
		
		/// Write \a data of length \a length to \a writeSocket
		/// \param[in] writeSocket The socket to write to
		/// \param[in] data The data to write
		/// \param[in] length The length of \a data	
		void Write( const SOCKET writeSocket, const char* data, const int length );
		
		/// Read data from a socket 
		/// \param[in] s the socket 
		/// \param[in] rakPeer The instance of rakPeer containing the recvFrom C callback
		/// \param[in] errorCode An error code if an error occured .
		/// \return Returns true if you successfully read data, false on error.
		int RecvFrom( const SOCKET s, RakPeer *rakPeer, int *errorCode );
		
	#if !defined(_COMPATIBILITY_1)
		/// Retrieve all local IP address in a string format.
		/// \param[in] ipList An array of ip address in dotted notation.
		void GetMyIP( char ipList[ 10 ][ 16 ] );
	#endif
		
		/// Call sendto (UDP obviously)
		/// \param[in] s the socket
		/// \param[in] data The byte buffer to send 
		/// \param[in] length The length of the \a data in bytes
		/// \param[in] ip The address of the remote host in dotted notation.
		/// \param[in] port The port number to send to.
		/// \return 0 on success, nonzero on failure.
		int SendTo( SOCKET s, const char *data, int length, char ip[ 16 ], unsigned short port );
		
		/// Call sendto (UDP obviously)
		/// \param[in] s the socket
		/// \param[in] data The byte buffer to send 
		/// \param[in] length The length of the \a data in bytes
		/// \param[in] binaryAddress The address of the remote host in binary format.
		/// \param[in] port The port number to send to.
		/// \return 0 on success, nonzero on failure.
		int SendTo( SOCKET s, const char *data, int length, unsigned int binaryAddress, unsigned short port );
			
		/// Returns the local port, useful when passing 0 as the startup port.
		/// \param[in] s The socket whose port we are referring to
		/// \return The local port
		unsigned short GetLocalPort ( SOCKET s );
	private:
		
		static bool socketLayerStarted;
	#ifdef _WIN32
		static WSADATA winsockInfo;
	#endif
		static SocketLayer *_instance;
	};
}

#endif

