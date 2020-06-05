/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/FastNodingValidator.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/algorithm/LineIntersector.h>


#include <memory> // for unique_ptr
#include <iostream>

namespace geos {
namespace noding { // geos.noding


void
ValidatingNoder::computeNodes(std::vector<SegmentString*>* segStrings)
{
    noder.computeNodes(segStrings);
    nodedSS = noder.getNodedSubstrings();
    validate();
}

void
ValidatingNoder::validate()
{
    FastNodingValidator nv(*nodedSS);
    nv.checkValid();
}

std::vector<SegmentString*>*
ValidatingNoder::getNodedSubstrings()
{
    return nodedSS;
}



} // namespace geos.noding
} // namespace geos
