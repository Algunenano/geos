/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/planargraph/NodeMap.h>
#include <geos/planargraph/Node.h>

#include <map>

using namespace std;

namespace geos {
namespace planargraph {

/**
 * Constructs a NodeMap without any Nodes.
 */
NodeMap::NodeMap()
{
}

NodeMap::~NodeMap()
{
}

NodeMap::container&
NodeMap::getNodeMap()
{
    return nodeMap;
}

/**
 * Adds a node to the map, replacing any that is already at that location.
 * @return the added node
 */
Node*
NodeMap::add(Node* n)
{
    nodeMap.insert(std::make_pair(std::make_pair(n->getCoordinate().x, n->getCoordinate().y), n));
    return n;
}

/**
 * Removes the Node at the given location, and returns it
 * (or null if no Node was there).
 */
Node*
NodeMap::remove(geom::Coordinate& pt)
{
	auto it = nodeMap.find(std::make_pair(pt.x, pt.y));
	auto r = it->second;
	nodeMap.erase(it);
	return r;
}

/* public */
void
NodeMap::getNodes(vector<Node*>& values)
{
    NodeMap::container::iterator it = nodeMap.begin(), itE = nodeMap.end();
    while(it != itE) {
        values.push_back(it->second);
        ++it;
    }
}

/**
 * Returns the Node at the given location, or null if no Node was there.
 */
Node*
NodeMap::find(const geom::Coordinate& coord)
{
    container::iterator found = nodeMap.find(std::make_pair(coord.x, coord.y));
    if(found == nodeMap.end()) {
        return nullptr;
    }
    else {
        return found->second;
    }
}

} //namespace planargraph
} //namespace geos

