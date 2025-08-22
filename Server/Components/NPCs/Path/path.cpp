/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "path.hpp"

NPCPath::NPCPath()
	: currentIndex_(0)
	, id_(-1)
{
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

	if (currentIndex_ >= points_.size() && !points_.empty())
	{
		currentIndex_ = points_.size() - 1;
	}
	else if (points_.empty())
	{
		currentIndex_ = 0;
	}

	return true;
}

void NPCPath::clear()
{
	points_.clear();
	currentIndex_ = 0;
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

const PathPoint* NPCPath::getNextPoint() const
{
	if (currentIndex_ >= points_.size())
	{
		return nullptr;
	}

	return &points_[currentIndex_];
}

bool NPCPath::hasNextPoint() const
{
	return currentIndex_ < points_.size();
}

void NPCPath::moveToNextPoint()
{
	if (currentIndex_ < points_.size())
	{
		currentIndex_++;
	}
}

void NPCPath::reset()
{
	currentIndex_ = 0;
}

void NPCPath::setCurrentIndex(size_t index)
{
	if (index <= points_.size())
	{
		currentIndex_ = index;
	}
}

size_t NPCPath::getCurrentIndex() const
{
	return currentIndex_;
}