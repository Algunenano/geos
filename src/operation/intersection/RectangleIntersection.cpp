/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mika Heiskanen <mika.heiskanen@fmi.fi>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/Orientation.h>
#include <geos/operation/intersection/RectangleIntersection.h>
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersectionBuilder.h>
#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/util/UnsupportedOperationException.h>
#include <list>
#include <stdexcept>

using geos::operation::intersection::Rectangle;
using geos::operation::intersection::RectangleIntersectionBuilder;
using namespace geos::geom;
using namespace geos::algorithm;
namespace geos {
namespace operation { // geos::operation
namespace intersection { // geos::operation::intersection

/**
 * \brief Test if two coordinates are different
 */

inline
bool
different(double x1, double y1, double x2, double y2)
{
    return !(x1 == x2 && y1 == y2);
}

/**
 * \brief Calculate a line intersection point
 *
 * Note:
 *   - Calling this with x1,y1 and x2,y2 swapped cuts the other end of the line
 *   - Calling this with x and y swapped cuts in y-direction instead
 *   - Calling with 1<->2 and x<->y swapped works too
 */

inline
void
clip_one_edge(double& x1, double& y1, double x2, double y2, double limit)
{
    if(x2 == limit) {
        y1 = y2;
        x1 = x2;
    }

    if(x1 != x2) {
        y1 += (y2 - y1) * (limit - x1) / (x2 - x1);
        x1 = limit;
    }
}

/**
 * \brief Start point is outside, endpoint is definitely inside
 *
 * Note: Even though one might think using >= etc operators would produce
 *       the same result, that is not the case. We rely on the fact
 *       that nothing is clipped unless the point is truly outside the
 *       rectangle! Without this handling lines ending on the edges of
 *       the rectangle would be very difficult.
 */

void
clip_to_edges(double& x1, double& y1,
              double x2, double y2,
              const Rectangle& rect)
{
    if(x1 < rect.xmin()) {
        clip_one_edge(x1, y1, x2, y2, rect.xmin());
    }
    else if(x1 > rect.xmax()) {
        clip_one_edge(x1, y1, x2, y2, rect.xmax());
    }

    if(y1 < rect.ymin()) {
        clip_one_edge(y1, x1, y2, x2, rect.ymin());
    }
    else if(y1 > rect.ymax()) {
        clip_one_edge(y1, x1, y2, x2, rect.ymax());
    }
}

/**
 * Clips the line defined by the 2 Coordinates
 * If required, it will modify the Coordinates to fall into the borders of the rectangle
 * @return True if the final line are inside, false if discarded
 *
 * Based on https://arxiv.org/abs/1908.01350
 * '''
 * Matthes, Dimitrios & Drakopoulos, Vasileios. (2019). Another Simple but Faster Method for 2D Line Clipping.
 * International Journal of Computer Graphics & Animation. 9. 1-15. 10.5121/ijcga.2019.9301.
 * '''
 */
int
clip_segment(Coordinate &p1, Coordinate &p2, const Rectangle &rect)
{
	double x[2] = {p1.x, p2.x};
	double y[2] = {p1.y, p2.y};
    double z[2] = {p1.z, p2.z};

	if (p1.x < rect.xmin() && p2.x < rect.xmin())
		return -1;
	if (p1.x > rect.xmax() && p2.x > rect.xmax())
		return -1;
	if (p1.y < rect.ymin() && p2.y < rect.ymin())
		return -1;
	if (p1.y > rect.ymax() && p2.y > rect.ymax())
		return -1;

	/* Note that we don't need to handle the division by 0 because it can't happen; it would mean the
	 * tile is fully outside (handled above) or fully inside (does not trigger the condition)
	 */
    int changes = 0;
	for (size_t i = 0; i < 2; i++)
	{
		if (x[i] < rect.xmin())
		{
			x[i] = rect.xmin();
			y[i] = ((p2.y - p1.y) / (p2.x - p1.x)) * (rect.xmin() - p1.x) + p1.y;
            z[i] = ((p2.z - p1.z) / (p2.x - p1.x)) * (rect.xmin() - p1.x) + p1.z;
            changes++;
		}
		else if (x[i] > rect.xmax())
		{
			x[i] = rect.xmax();
			y[i] = ((p2.y - p1.y) / (p2.x - p1.x)) * (rect.xmax() - p1.x) + p1.y;
            z[i] = ((p2.z - p1.z) / (p2.x - p1.x)) * (rect.xmax() - p1.x) + p1.z;
            changes++;
		}

		if (y[i] < rect.ymin())
		{
			y[i] = rect.ymin();
			x[i] = ((p2.x - p1.x) / (p2.y - p1.y)) * (rect.ymin() - p1.y) + p1.x;
            z[i] = ((p2.z - p1.z) / (p2.y - p1.y)) * (rect.ymin() - p1.y) + p1.z;
            changes++;
		}
		else if (y[i] > rect.ymax())
		{
			y[i] = rect.ymax();
			x[i] = ((p2.x - p1.x) / (p2.y - p1.y)) * (rect.ymax() - p1.y) + p1.x;
            z[i] = ((p2.z - p1.z) / (p2.y - p1.y)) * (rect.ymax() - p1.y) + p1.z;
            changes++;
		}
	}

	if (!(x[0] < rect.xmin() && x[1] < rect.xmin()) && !(x[0] > rect.xmax() && x[1] > rect.xmax()))
    {
        p1.x = x[0];
        p1.y = y[0];
        p1.z = z[0];
        p2.x = x[1];
        p2.y = y[1];
        p2.z = z[1];

        return changes;
    }

    return -1;

}


/**
 * \brief Clip  geometry
 *
 * Here outGeom may also be a MultiPoint
 */

void
RectangleIntersection::clip_point(const geom::Point* g,
                                  RectangleIntersectionBuilder& parts,
                                  const Rectangle& rect)
{
    if(g == nullptr) {
        return;
    }

    double x = g->getX();
    double y = g->getY();

    if(rect.position(x, y) == Rectangle::Inside) {
        parts.add(dynamic_cast<geom::Point*>(g->clone().release()));
    }
}

bool
RectangleIntersection::clip_linestring_parts(const geom::LineString* gi,
        RectangleIntersectionBuilder& parts,
        const Rectangle& rect)
{
    if (!gi) {
        return false;
    }

    auto n = gi->getNumPoints();
    if (n < 1) {
        return false;
    }

    std::vector<Coordinate> cs;
    gi->getCoordinatesRO()->toVector(cs);

    std::vector<Coordinate> stored_coordinates{};

    int changes = 0;
    for (size_t i = 1; i < n; i++) {
        Coordinate p1(cs[i - 1].x, cs[i - 1].y, cs[i - 1].z);
        Coordinate p2(cs[i].x, cs[i].y, cs[i].z);

//        std::cout << "Segment: " << p1 << "  ---  " << p2 << std::endl;

        int segment_changes = clip_segment(p1, p2, rect);
        if (segment_changes == -1) {
            changes++;
            continue;
        }

        changes += segment_changes;
//            std::cout << "Clipped: " << p1 << "  ---  " << p2 << std::endl;
        if (!stored_coordinates.size() || !stored_coordinates.back().equals2D(p1)) {
            if (stored_coordinates.size() > 1) {
                std::vector<Coordinate> new_vector(stored_coordinates);
                auto seq = _csf->create(std::move(new_vector));
                geom::LineString* line = _gf->createLineString(seq.release());
                parts.add(line);
            }
            stored_coordinates.clear();
            stored_coordinates.push_back(p1);
        }
        if (!p1.equals2D(p2)) {
            stored_coordinates.push_back(p2);
        }
    }

    if (changes == 0) {
        return true;
    }

    if (stored_coordinates.size() > 1)
    {
        std::vector<Coordinate> new_vector(stored_coordinates);
        auto seq = _csf->create(std::move(new_vector));
        geom::LineString* line = _gf->createLineString(seq.release());
        parts.add(line);
    }
    return false;

}

/**
 * \brief Clip polygon, do not close clipped ones
 */

void
RectangleIntersection::clip_polygon_to_linestrings(const geom::Polygon* g,
        RectangleIntersectionBuilder& toParts,
        const Rectangle& rect)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    // Clip the exterior first to see what's going on

    RectangleIntersectionBuilder parts(*_gf);

    // If everything was in, just clone the original

    if(clip_linestring_parts(g->getExteriorRing(), parts, rect)) {
        toParts.add(dynamic_cast<geom::Polygon*>(g->clone().release()));
        return;
    }

    // Now, if parts is empty, our rectangle may be inside the polygon
    // If not, holes are outside too

    if(parts.empty()) {
        // We could now check whether the rectangle is inside the outer
        // ring to avoid checking the holes. However, if holes are much
        // smaller than the exterior ring just checking the holes
        // separately could be faster.

        if(g->getNumInteriorRing() == 0) {
            return;
        }

    }
    else {
        // The exterior must have been clipped into linestrings.
        // Move them to the actual parts collector, clearing parts.

        parts.reconnect();
        parts.release(toParts);
    }

    // Handle the holes now:
    // - Clipped ones become linestrings
    // - Intact ones become new polygons without holes

    for(size_t i = 0, n = g->getNumInteriorRing(); i < n; ++i) {
        if(clip_linestring_parts(g->getInteriorRingN(i), parts, rect)) {
            auto &c = g->getInteriorRingN(i)->getCoordinatesRO()->front();
            if (rect.onEdge(rect.position(c.x, c.y))) {
                // Matches the rectangle boundaries exactly
                return;
            }
            // clones
            LinearRing* hole = new LinearRing(*(g->getInteriorRingN(i)));
            // becomes exterior
            Polygon* poly = _gf->createPolygon(hole, nullptr);
            toParts.add(poly);
        }
        else if(!parts.empty()) {
            parts.reconnect();
            parts.release(toParts);
        }
    }
}

/**
 * \brief Clip polygon, close clipped ones
 */

void
RectangleIntersection::clip_polygon_to_polygons(const geom::Polygon* g,
        RectangleIntersectionBuilder& toParts,
        const Rectangle& rect)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    // Clip the exterior first to see what's going on

    RectangleIntersectionBuilder parts(*_gf);

    // If everything was in, just clone the original

    const LineString* shell = g->getExteriorRing();
    if(clip_linestring_parts(shell, parts, rect)) {
        toParts.add(dynamic_cast<geom::Polygon*>(g->clone().release()));
        return;
    }

    // If there were no intersections, the outer ring might be
    // completely outside.

    using geos::algorithm::Orientation;
    if(parts.empty()) {
        Coordinate rectCenter(rect.xmin(), rect.ymin());
        rectCenter.x += (rect.xmax() - rect.xmin()) / 2;
        rectCenter.y += (rect.ymax() - rect.ymin()) / 2;
        if(PointLocation::locateInRing(rectCenter,
                                       *g->getExteriorRing()->getCoordinatesRO())
                != Location::INTERIOR) {
            return;
        }
    }
    else {
        // TODO: make CCW checking part of clip_linestring_parts ?
        if(Orientation::isCCW(shell->getCoordinatesRO())) {
            parts.reverseLines();
        }
    }

    // Must do this to make sure all end points are on the edges

    parts.reconnect();

    // Handle the holes now:
    // - Clipped ones become part of the exterior
    // - Intact ones become holes in new polygons formed by exterior parts


    for(size_t i = 0, n = g->getNumInteriorRing(); i < n; ++i) {
        RectangleIntersectionBuilder holeparts(*_gf);
        const LinearRing* hole = g->getInteriorRingN(i);
        if(clip_linestring_parts(hole, holeparts, rect)) {
            auto &c = hole->getCoordinatesRO()->front();
            if (rect.onEdge(rect.position(c.x, c.y))) {
                // Matches the rectangle boundaries exactly
                return;
            }

            // becomes exterior
            LinearRing* cloned = new LinearRing(*hole);
            Polygon* poly = _gf->createPolygon(cloned, nullptr);
            parts.add(poly);
        }
        else {
            if(!holeparts.empty()) {
                // TODO: make CCW checking part of clip_linestring_parts ?
                if(! Orientation::isCCW(hole->getCoordinatesRO())) {
                    holeparts.reverseLines();
                }
                holeparts.reconnect();
                holeparts.release(parts);
            }
            else {

                Coordinate rectCenter(rect.xmin(), rect.ymin());
                rectCenter.x += (rect.xmax() - rect.xmin()) / 2;
                rectCenter.y += (rect.ymax() - rect.ymin()) / 2;
                if(PointLocation::isInRing(rectCenter,
                                           g->getInteriorRingN(i)->getCoordinatesRO())) {
                    // Completely inside the hole
                    return;
                }
            }
        }
    }

    parts.reconnectPolygons(rect);
    parts.release(toParts);

}

/**
 * \brief Clip  geometry
 */

void
RectangleIntersection::clip_polygon(const geom::Polygon* g,
                                    RectangleIntersectionBuilder& parts,
                                    const Rectangle& rect,
                                    bool keep_polygons)
{
    if(keep_polygons) {
        clip_polygon_to_polygons(g, parts, rect);
    }
    else {
        clip_polygon_to_linestrings(g, parts, rect);
    }
}

/**
 * \brief Clip geometry
 */

void
RectangleIntersection::clip_linestring(const geom::LineString* g,
                                       RectangleIntersectionBuilder& parts,
                                       const Rectangle& rect)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    // If everything was in, just clone the original

    if(clip_linestring_parts(g, parts, rect)) {
        parts.add(dynamic_cast<geom::LineString*>(g->clone().release()));
    }

}

void
RectangleIntersection::clip_multipoint(const geom::MultiPoint* g,
                                       RectangleIntersectionBuilder& parts,
                                       const Rectangle& rect)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }
    for(size_t i = 0, n = g->getNumGeometries(); i < n; ++i) {
        clip_point(g->getGeometryN(i), parts, rect);
    }
}

void
RectangleIntersection::clip_multilinestring(const geom::MultiLineString* g,
        RectangleIntersectionBuilder& parts,
        const Rectangle& rect)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    for(size_t i = 0, n = g->getNumGeometries(); i < n; ++i) {
        clip_linestring(g->getGeometryN(i), parts, rect);
    }
}

void
RectangleIntersection::clip_multipolygon(const geom::MultiPolygon* g,
        RectangleIntersectionBuilder& parts,
        const Rectangle& rect,
        bool keep_polygons)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    for(size_t i = 0, n = g->getNumGeometries(); i < n; ++i) {
        clip_polygon(g->getGeometryN(i), parts, rect, keep_polygons);
    }
}

void
RectangleIntersection::clip_geometrycollection(
    const geom::GeometryCollection* g,
    RectangleIntersectionBuilder& parts,
    const Rectangle& rect,
    bool keep_polygons)
{
    if(g == nullptr || g->isEmpty()) {
        return;
    }

    for(size_t i = 0, n = g->getNumGeometries(); i < n; ++i) {
        clip_geom(g->getGeometryN(i),
                  parts, rect, keep_polygons);
    }
}

void
RectangleIntersection::clip_geom(const geom::Geometry* g,
                                 RectangleIntersectionBuilder& parts,
                                 const Rectangle& rect,
                                 bool keep_polygons)
{
    if(const Point* p1 = dynamic_cast<const geom::Point*>(g)) {
        return clip_point(p1, parts, rect);
    }
    else if(const MultiPoint* p2 = dynamic_cast<const geom::MultiPoint*>(g)) {
        return clip_multipoint(p2, parts, rect);
    }
    else if(const LineString* p3 = dynamic_cast<const geom::LineString*>(g)) {
        return clip_linestring(p3, parts, rect);
    }
    else if(const MultiLineString* p4 = dynamic_cast<const geom::MultiLineString*>(g)) {
        return clip_multilinestring(p4, parts, rect);
    }
    else if(const Polygon* p5 = dynamic_cast<const geom::Polygon*>(g)) {
        return clip_polygon(p5, parts, rect, keep_polygons);
    }
    else if(const MultiPolygon* p6 = dynamic_cast<const geom::MultiPolygon*>(g)) {
        return clip_multipolygon(p6, parts, rect, keep_polygons);
    }
    else if(const GeometryCollection* p7 = dynamic_cast<const geom::GeometryCollection*>(g)) {
        return clip_geometrycollection(p7, parts, rect, keep_polygons);
    }
    else {
        throw util::UnsupportedOperationException("Encountered an unknown geometry component when clipping polygons");
    }
}

/* public static */
std::unique_ptr<geom::Geometry>
RectangleIntersection::clipBoundary(const geom::Geometry& g, const Rectangle& rect)
{
    RectangleIntersection ri(g, rect);
    return ri.clipBoundary();
}

std::unique_ptr<geom::Geometry>
RectangleIntersection::clipBoundary()
{
    RectangleIntersectionBuilder parts(*_gf);

    bool keep_polygons = false;
    clip_geom(&_geom, parts, _rect, keep_polygons);

    return parts.build();
}

/* public static */
std::unique_ptr<geom::Geometry>
RectangleIntersection::clip(const geom::Geometry& g, const Rectangle& rect)
{
    RectangleIntersection ri(g, rect);
    return ri.clip();
}

std::unique_ptr<geom::Geometry>
RectangleIntersection::clip()
{
    RectangleIntersectionBuilder parts(*_gf);

    bool keep_polygons = true;
    clip_geom(&_geom, parts, _rect, keep_polygons);

    return parts.build();
}

RectangleIntersection::RectangleIntersection(const geom::Geometry& geom, const Rectangle& rect)
    : _geom(geom), _rect(rect),
      _gf(geom.getFactory())
{
    _csf = _gf->getCoordinateSequenceFactory();
}

} // namespace geos::operation::intersection
} // namespace geos::operation
} // namespace geos
