/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#include "path.hpp"

NPCPath::NPCPath()
	: id_(-1)
{
	// Reserve default capacity to reduce reallocations
	points_.reserve(DEFAULT_CAPACITY);
}

void NPCPath::reserve(size_t capacity)
{
	points_.reserve(capacity);
}

void NPCPath::addPoint(const Vector3& position, float stopRange)
{
	points_.emplace_back(position, stopRange);
}

void NPCPath::addPoint(float x, float y, float z, float stopRange)
{
	points_.emplace_back(Vector3 { x, y, z }, stopRange);
}

bool NPCPath::removePoint(size_t index)
{
	if (index >= points_.size())
	{
		return false;
	}

	points_.erase(points_.begin() + index);
	return true;
}

void NPCPath::clear()
{
	points_.clear();
}

size_t NPCPath::getPointCount() const
{
	return points_.size();
}

const PathPoint* NPCPath::getPoint(size_t index) const
{
	if (index >= points_.size())
	{
		return nullptr;
	}

	return &points_[index];
}
