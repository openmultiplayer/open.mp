/// \file
/// \brief Contains enumerations used by the ReplicaManager system.  This file is a lightweight header, so you can include it without worrying about linking in lots of other crap
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

#ifndef __REPLICA_ENUMS_H
#define __REPLICA_ENUMS_H

namespace RakNet
{
	/// Replica interface flags, used to enable and disable function calls on the Replica object
	/// Passed to ReplicaManager::EnableReplicaInterfaces and ReplicaManager::DisableReplicaInterfaces
	enum
	{
		REPLICA_RECEIVE_DESTRUCTION=1<<0,
		REPLICA_RECEIVE_SERIALIZE=1<<1,
		REPLICA_RECEIVE_SCOPE_CHANGE=1<<2,
		REPLICA_SEND_CONSTRUCTION=1<<3,
		REPLICA_SEND_DESTRUCTION=1<<4,
		REPLICA_SEND_SCOPE_CHANGE=1<<5,
		REPLICA_SEND_SERIALIZE=1<<6,
		REPLICA_SET_ALL = 0xFF // Allow all of the above
	};

	enum ReplicaReturnResult
	{
		/// This means call the function again later, with the same parameters
		REPLICA_PROCESS_LATER,
		/// This means we are done processing (the normal result to return)
		REPLICA_PROCESSING_DONE,
		/// This means cancel the processing - don't send any network messages and don't change the current state.
		REPLICA_CANCEL_PROCESS,
		/// Same as REPLICA_PROCESSING_DONE, where a message is sent, but does not clear the send bit.
		/// Useful for multi-part sends with different reliability levels.
		/// Only currently used by Replica::Serialize
		REPLICA_PROCESS_AGAIN,
	};
}

#endif
