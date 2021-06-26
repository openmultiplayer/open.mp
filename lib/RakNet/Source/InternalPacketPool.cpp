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

#include "InternalPacketPool.h"
#include <assert.h>

using namespace RakNet;

InternalPacketPool::InternalPacketPool()
{	
	// Speed things up by not reallocating at runtime when a mutex is locked.
	pool.ClearAndForceAllocation( 64 );
	unsigned i;
	for (i=0; i < 64; i++)
		pool.Push(new InternalPacket);
}

InternalPacketPool::~InternalPacketPool()
{
	ClearPool();
}

void InternalPacketPool::ClearPool( void )
{
	while ( pool.Size() )
		delete pool.Pop();
}
/*
InternalPacket* InternalPacketPool::GetPointer( void )
{
	if ( pool.Size() )
		return pool.Pop();
	return new InternalPacket;

}
*/

void InternalPacketPool::ReleasePointer( InternalPacket *p )
{
	if ( p == 0 )
	{
		// Releasing a null pointer?

		RakAssert( 0 );

		return ;
	}
	
#ifdef _DEBUG
	p->data=0;
#endif
	//poolMutex.Lock();
	pool.Push( p );
	//poolMutex.Unlock();
}

