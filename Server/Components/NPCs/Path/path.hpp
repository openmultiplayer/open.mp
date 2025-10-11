/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>
#include <Impl/pool_impl.hpp>
#include <vector>

struct PathPoint
{
	Vector3 position;
	float stopRange;

	PathPoint(const Vector3& pos, float range = 1.0f)
		: position(pos)
		, stopRange(range)
	{
	}
};

class NPCPath : public NoCopy
{
public:
	NPCPath();

	// Reserve capacity for better performance
	void reserve(size_t capacity);

	void addPoint(const Vector3& position, float stopRange = 1.0f);

	void addPoint(float x, float y, float z, float stopRange = 1.0f);

	bool removePoint(size_t index);

	void clear();

	size_t getPointCount() const;

	const PathPoint* getPoint(size_t index) const;

	int getID() const
	{
		return id_;
	}

	void setID(int id)
	{
		id_ = id;
	}

	const std::vector<PathPoint>& getPoints() const
	{
		return points_;
	}

private:
	std::vector<PathPoint> points_;
	int id_;

	// Pre-allocated capacity to reduce reallocations
	static constexpr size_t DEFAULT_CAPACITY = 16;
};