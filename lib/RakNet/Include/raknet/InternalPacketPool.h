/// \file
/// \brief \b [Internal] Memory pool for InternalPacket*
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

#ifndef __INTERNAL_PACKET_POOL
#define __INTERNAL_PACKET_POOL
#include "DS_Queue.h"
#include "InternalPacket.h"

namespace RakNet
{
	/// Handles of a pool of InternalPacket pointers.  This is only here for efficiency.
	/// \sa InternalPacket.h
	class InternalPacketPool
	{
	public:
		
		/// Constructor	
		InternalPacketPool();
		
		/// Destructor	
		~InternalPacketPool();
		
		/// Get an InternalPacket pointer.  Will either allocate a new one or return one from the pool
		/// \return An InternalPacket pointer.
		InternalPacket* GetPointer( void )
		{if ( pool.Size() )
			return pool.Pop();
		return new InternalPacket;}
		
		/// Return an InternalPacket pointer to the pool.
		/// \param[in] p A pointer to an InternalPacket you no longer need.
		void ReleasePointer( InternalPacket *p );
		
		// Delete all InternalPacket pointers in the pool.	
		void ClearPool( void );

	private:
		/// Queue of internal packets
		DataStructures::Queue<InternalPacket*> pool;
	};
}

#endif

