/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include "path.hpp"
#include <vector>
#include <memory>
#include <queue>

class NPCPathPool
{
public:
	NPCPathPool();
	~NPCPathPool();

	NPCPath* create();

	void destroy(NPCPath* path);

	NPCPath* get(int pathId);

	void destroyAll();

	size_t getPathCount() const;

private:
	std::vector<std::unique_ptr<NPCPath>> paths_;
	std::queue<int> freeIds_;
	int nextId_;
	size_t activePathCount_; // Track active paths for faster getPathCount()
};