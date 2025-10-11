/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include "node.hpp"
#include <array>
#include <memory>

constexpr int MAX_NODES = 64;

class NPCNodeManager
{
public:
	NPCNodeManager();
	~NPCNodeManager();

	bool isNodeOpen(int nodeId) const;
	NPCNode* getNode(int nodeId);

	bool openNode(ICore* core, int nodeId);
	void closeNode(int nodeId);
	void closeAllNodes();

private:
	StaticArray<bool, MAX_NODES> opened_;
	StaticArray<std::unique_ptr<NPCNode>, MAX_NODES> nodes_;
};