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

#include <geos/operation/intersection/Rectangle.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>

namespace geos {
namespace operation { // geos::operation
namespace intersection { // geos::operation::intersection

/*
 * Create a clipping rectangle
 */

Rectangle::Rectangle(double x1, double y1, double x2, double y2)
    : xMin(x1)
    , yMin(y1)
    , xMax(x2)
    , yMax(y2)
{
    if(xMin >= xMax || yMin >= yMax) {
        throw util::IllegalArgumentException("Clipping rectangle must be non-empty");
    }
}

geom::Polygon*
Rectangle::toPolygon(const geom::GeometryFactory& f, double zValue) const
{
    geom::LinearRing* ls = toLinearRing(f, zValue);
    return f.createPolygon(ls, nullptr);
}

geom::LinearRing*
Rectangle::toLinearRing(const geom::GeometryFactory& f, double zValue) const
{
    const geom::CoordinateSequenceFactory* csf = f.getCoordinateSequenceFactory();
    auto seq = csf->create(5, zValue != DoubleNotANumber ? 3 : 2);
    seq->setAt(geom::Coordinate(xMin, yMin, zValue), 0);
    seq->setAt(geom::Coordinate(xMin, yMax, zValue), 1);
    seq->setAt(geom::Coordinate(xMax, yMax, zValue), 2);
    seq->setAt(geom::Coordinate(xMax, yMin, zValue), 3);
    seq->setAt(seq->getAt(0), 4); // close
    return f.createLinearRing(seq.release());
}

} // namespace geos::operation::intersection
} // namespace geos::operation
} // namespace geos
