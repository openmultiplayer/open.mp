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

#include "RPCMap.h"
#include <string.h>

using namespace RakNet;

RPCMap::RPCMap()
{
}
RPCMap::~RPCMap()
{
	Clear();
}
void RPCMap::Clear(void)
{
	unsigned i;
	RPCNode *node;
	for (i=0; i < rpcSet.Size(); i++)
	{
		node=rpcSet[i];
		if (node)
		{
#if RPCID_STRING
			delete [] node->uniqueIdentifier;
#endif
			delete node;
		}
	}
	rpcSet.Clear();
}
RPCNode *RPCMap::GetNodeFromIndex(RPCIndex index)
{
	if ((unsigned)index < rpcSet.Size())
		return rpcSet[(unsigned)index];
	return 0;
}
RPCNode *RPCMap::GetNodeFromFunctionName(RPCID uniqueIdentifier)
{
	unsigned index;
	index=(unsigned)GetIndexFromFunctionName(uniqueIdentifier);
	if ((RPCIndex)index!=UNDEFINED_RPC_INDEX)
		return rpcSet[index];
	return 0;
}
RPCIndex RPCMap::GetIndexFromFunctionName(RPCID uniqueIdentifier)
{
	unsigned index;
	for (index=0; index < rpcSet.Size(); index++)
		if (rpcSet[index] && 
#if RPCID_STRING
			strcmp(rpcSet[index]->uniqueIdentifier, uniqueIdentifier)==0
#else
			rpcSet[index]->uniqueIdentifier==uniqueIdentifier
#endif
		)
			return (RPCIndex) index;
	return UNDEFINED_RPC_INDEX;
}

// Called from the user thread for the local system
void RPCMap::AddIdentifierWithFunction(RPCID uniqueIdentifier, void *functionPointer, bool isPointerToMember, void* extraPointer)
{
#ifdef _DEBUG
	RakAssert(rpcSet.Size()+1 < MAX_RPC_MAP_SIZE); // If this hits change the typedef of RPCIndex to use an unsigned short
	RakAssert(uniqueIdentifier);
#if RPCID_STRING
	RakAssert(uniqueIdentifier[0]);
#endif
	RakAssert(functionPointer);
#endif

	unsigned index, existingNodeIndex;
	RPCNode *node;

	existingNodeIndex=GetIndexFromFunctionName(uniqueIdentifier);
	if ((RPCIndex)existingNodeIndex!=UNDEFINED_RPC_INDEX) // Insert at any free spot.
	{
		// Trying to insert an identifier at any free slot and that identifier already exists
		// The user should not insert nodes that already exist in the list

//		RakAssert(0);

		return;
	}

	node = new RPCNode;
#if RPCID_STRING
	node->uniqueIdentifier = new char [strlen(uniqueIdentifier)+1];
	strcpy(node->uniqueIdentifier, uniqueIdentifier);
#else
	node->uniqueIdentifier = uniqueIdentifier;
#endif
	node->functionPointer=functionPointer;
	node->isPointerToMember=isPointerToMember;
	node->extraPointer=extraPointer;

	// Insert into an empty spot if possible
	for (index=0; index < rpcSet.Size(); index++)
	{
		if (rpcSet[index]==0)
		{
			rpcSet.Replace(node, 0, index);
			return;
		}
	}

	rpcSet.Insert(node); // No empty spots available so just add to the end of the list

}
void RPCMap::AddIdentifierAtIndex(RPCID uniqueIdentifier, RPCIndex insertionIndex)
{

	RakAssert(uniqueIdentifier);
#if RPCID_STRING
	RakAssert(uniqueIdentifier[0]);
#endif


	unsigned existingNodeIndex;
	RPCNode *node, *oldNode;

	existingNodeIndex=GetIndexFromFunctionName(uniqueIdentifier);

	if (existingNodeIndex==insertionIndex)
		return; // Already there

	if ((RPCIndex)existingNodeIndex!=UNDEFINED_RPC_INDEX)
	{
		// Delete the existing one
		oldNode=rpcSet[existingNodeIndex];
		rpcSet[existingNodeIndex]=0;
#if RPCID_STRING
		delete [] oldNode->uniqueIdentifier;
#endif
		delete oldNode;
	}

	node = new RPCNode;
#if RPCID_STRING
	node->uniqueIdentifier = new char [strlen(uniqueIdentifier)+1];
	strcpy(node->uniqueIdentifier, uniqueIdentifier);
#else
	node->uniqueIdentifier = uniqueIdentifier;
#endif
	node->functionPointer=0;

	// Insert at a user specified spot
	if (insertionIndex < rpcSet.Size())
	{
		// Overwrite what is there already
		oldNode=rpcSet[insertionIndex];
		if (oldNode)
		{
#if RPCID_STRING
			delete [] oldNode->uniqueIdentifier;
#endif
			delete oldNode;
		}
		rpcSet[insertionIndex]=node;
	}
	else
	{
		// Insert after the end of the list and use 0 as a filler for the empty spots
		rpcSet.Replace(node, 0, insertionIndex);
	}
}

void RPCMap::RemoveNode(RPCID uniqueIdentifier)
{
	unsigned index;
	index=GetIndexFromFunctionName(uniqueIdentifier);
    
	RakAssert(index!=UNDEFINED_RPC_INDEX); // If this hits then the user was removing an RPC call that wasn't currently registered
	
	RPCNode *node;
	node = rpcSet[index];
#if RPCID_STRING
	delete [] node->uniqueIdentifier;
#endif
	delete node;
	rpcSet[index]=0;
}

