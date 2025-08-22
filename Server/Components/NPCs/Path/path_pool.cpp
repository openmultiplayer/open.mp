/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "path_pool.hpp"

NPCPathPool::NPCPathPool()
	: nextId_(0)
{
}

NPCPathPool::~NPCPathPool()
{
	destroyAll();
}

NPCPath* NPCPathPool::create()
{
	int id;

	// Reuse freed IDs if available
	if (!freeIds_.empty())
	{
		id = freeIds_.front();
		freeIds_.pop();
	}
	else
	{
		id = nextId_++;
	}

	// Ensure vector is large enough
	if (static_cast<size_t>(id) >= paths_.size())
	{
		paths_.resize(id + 1);
	}

	// Create new path
	auto path = std::make_unique<NPCPath>();
	path->setID(id);

	NPCPath* pathPtr = path.get();
	paths_[id] = std::move(path);

	return pathPtr;
}

void NPCPathPool::destroy(NPCPath* path)
{
	if (!path)
		return;

	int id = path->getID();
	if (id >= 0 && static_cast<size_t>(id) < paths_.size() && paths_[id])
	{
		paths_[id].reset();
		freeIds_.push(id);
	}
}

NPCPath* NPCPathPool::get(int pathId)
{
	if (pathId >= 0 && static_cast<size_t>(pathId) < paths_.size())
	{
		return paths_[pathId].get();
	}
	return nullptr;
}

void NPCPathPool::destroyAll()
{
	paths_.clear();
	while (!freeIds_.empty())
	{
		freeIds_.pop();
	}
	nextId_ = 0;
}

size_t NPCPathPool::getPathCount() const
{
	size_t count = 0;
	for (const auto& path : paths_)
	{
		if (path)
		{
			count++;
		}
	}
	return count;
}