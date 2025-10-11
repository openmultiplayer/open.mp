/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#include "node_manager.hpp"

NPCNodeManager::NPCNodeManager()
{
	opened_.fill(false);
}

NPCNodeManager::~NPCNodeManager()
{
	closeAllNodes();
}

bool NPCNodeManager::isNodeOpen(int nodeId) const
{
	if (nodeId < 0 || nodeId >= MAX_NODES)
	{
		return false;
	}
	return opened_[nodeId];
}

NPCNode* NPCNodeManager::getNode(int nodeId)
{
	if (nodeId < 0 || nodeId >= MAX_NODES)
	{
		return nullptr;
	}
	return nodes_[nodeId].get();
}

bool NPCNodeManager::openNode(ICore* core, int nodeId)
{
	if (nodeId < 0 || nodeId >= MAX_NODES)
	{
		return false;
	}

	if (opened_[nodeId])
	{
		return false;
	}

	nodes_[nodeId] = std::make_unique<NPCNode>(nodeId);

	if (!nodes_[nodeId]->initialize(core))
	{
		nodes_[nodeId].reset();
		return false;
	}
	opened_[nodeId] = true;
	return true;
}

void NPCNodeManager::closeNode(int nodeId)
{
	if (nodeId < 0 || nodeId >= MAX_NODES)
	{
		return;
	}

	if (!opened_[nodeId])
	{
		return;
	}
	nodes_[nodeId].reset();
	opened_[nodeId] = false;
}

void NPCNodeManager::closeAllNodes()
{
	for (int i = 0; i < MAX_NODES; ++i)
	{
		if (opened_[i])
		{
			closeNode(i);
		}
	}
}