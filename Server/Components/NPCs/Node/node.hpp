/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include <sdk.hpp>
#include <fstream>

class NPC;
using namespace Impl;

#pragma pack(push, 1)

struct NodeHeader
{
	uint32_t nodesNumber;
	uint32_t vehicleNodesNumber;
	uint32_t pedNodesNumber;
	uint32_t naviNodesNumber;
	uint32_t linksNumber;
};

struct PathNode
{
	uint32_t memAddress;
	uint32_t unknown1;
	int16_t positionX;
	int16_t positionY;
	int16_t positionZ;
	uint16_t unknown2;
	uint16_t linkId;
	uint16_t areaId;
	uint16_t nodeId;
	uint8_t pathWidth;
	uint8_t nodeType;
	uint32_t flags;
};

struct NaviNode
{
	int16_t positionX;
	int16_t positionY;
	uint16_t areaId;
	uint16_t nodeId;
	uint8_t directionX;
	uint8_t directionY;
	uint32_t flags;
};

struct LinkNode
{
	uint16_t areaId;
	uint16_t nodeId;
};

#pragma pack(pop)

class NPCNode
{
public:
	NPCNode(int nodeId);
	~NPCNode();

	bool initialize(ICore* core);
	uint16_t process(NPC* npc, uint16_t pointId, uint16_t lastPoint, uint16_t& currentLinkId);
	uint16_t processNodeChange(NPC* npc, uint16_t targetPointId);

	Vector3 getPosition();
	int getNodesNumber() const;
	void getHeaderInfo(uint32_t& vehicleNodes, uint32_t& pedNodes, uint32_t& naviNodes) const;

	int getNodeId() const;
	uint16_t getLinkId() const;
	uint16_t getAreaId() const;
	uint16_t getPointId() const;
	uint16_t getLinkCount() const;
	uint8_t getPathWidth() const;
	uint8_t getNodeType() const;
	uint16_t getLinkPoint() const;

	bool setLink(uint16_t linkId);
	bool setPoint(uint16_t pointId);

	uint16_t getLastLinkTargetNodeId() const;
	uint16_t getLastLinkTargetPointId() const;

private:
	int nodeId_;
	bool initialized_;

	NodeHeader nodeHeader_;
	DynamicArray<PathNode> pathNodes_;
	DynamicArray<NaviNode> naviNodes_;
	DynamicArray<LinkNode> linkNodes_;

	uint16_t currentPointId_;
	uint16_t currentLinkId_;
};