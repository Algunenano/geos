//
// Test Suite for C-API GEOSUnaryUnion

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiunaryunion_data {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capiunaryunion_data()
        : geom1_(nullptr), geom2_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    std::string
    toWKT(GEOSGeometry* g)
    {
        char* wkt = GEOSWKTWriter_write(wktw_, g);
        std::string ret(wkt);
        GEOSFree(wkt);
        return ret;
    }

    ~test_capiunaryunion_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSWKTWriter_destroy(wktw_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capiunaryunion_data> group;
typedef group::object object;

group test_capiunaryunion_group("capi::GEOSUnaryUnion");

//
// Test Cases
//


// Self-union an empty point
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT EMPTY");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string("POINT EMPTY"));
}

// Self-union a 2d point
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POINT (6 3)");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string("POINT (6 3)"));
}

// Self-union a 3d point
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POINT (4 5 6)");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string("POINT Z (4 5 6)"));
}

// Self-union a multipoint with duplicated points
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("MULTIPOINT (4 5, 6 7, 4 5, 6 5, 6 7)");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string("MULTIPOINT (4 5, 6 5, 6 7)"));
}

// Self-union a collection of puntal and lineal geometries
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT(4 5), MULTIPOINT(6 7, 6 5, 6 7), LINESTRING(0 5, 10 5), LINESTRING(4 -10, 4 10))");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    GEOSGeometry* expected = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT (6 7), LINESTRING (4 -10, 4 5), LINESTRING (4 5, 4 10), LINESTRING (0 5, 4 5), LINESTRING (4 5, 10 5))");

    ensure(GEOSEquals(geom2_, expected) != 0);

    GEOSGeom_destroy(expected);
}

// Self-union a collection of puntal and polygonal geometries
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT(4 5), MULTIPOINT(6 7, 6 5, 6 7), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)))");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string(
                      "GEOMETRYCOLLECTION (POINT (6 7), POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (5 6, 7 6, 7 8, 5 8, 5 6)))"
                  ));
}

// Self-union a collection of lineal and polygonal geometries
template<>
template<>
void object::test<7>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (MULTILINESTRING((5 7, 12 7), (4 5, 6 5), (5.5 7.5, 6.5 7.5)), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)))");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    GEOSGeometry* expected = GEOSGeomFromWKT("GEOMETRYCOLLECTION (LINESTRING (5 7, 7 7), LINESTRING (10 7, 12 7), LINESTRING (5.5 7.5, 6.5 7.5), POLYGON ((10 7, 10 0, 0 0, 0 10, 10 10, 10 7), (5 6, 7 6, 7 7, 7 8, 5 8, 5 7, 5 6)))");

    ensure(GEOSEquals(geom2_, expected) != 0);

    GEOSGeom_destroy(expected);
}

// Self-union a collection of puntal, lineal and polygonal geometries
template<>
template<>
void object::test<8>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (MULTILINESTRING((5 7, 12 7), (4 5, 6 5), (5.5 7.5, 6.5 7.5)), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)), MULTIPOINT(6 6.5, 6 1, 12 2, 6 1))");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string(
                      "GEOMETRYCOLLECTION (POINT (6 6.5), POINT (12 2), LINESTRING (5 7, 7 7), LINESTRING (10 7, 12 7), LINESTRING (5.5 7.5, 6.5 7.5), POLYGON ((10 7, 10 0, 0 0, 0 10, 10 10, 10 7), (5 6, 7 6, 7 7, 7 8, 5 8, 5 7, 5 6)))"
                  ));
}

// Self-union a geometry with NaN coordinates
// See http://trac.osgeo.org/geos/ticket/530
template<>
template<>
void object::test<9>
()
{
    const char* hexwkb = "010200000002000000000000000000F8FF000000000000F8FF0000000000000000000000000000F03F";
    geom1_ = GEOSGeomFromHEX_buf((const unsigned char*)hexwkb, std::strlen(hexwkb));
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr == geom2_);

}

// Self-union an empty linestring
template<>
template<>
void object::test<10>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);

    ensure_equals(toWKT(geom2_), std::string("LINESTRING EMPTY"));
}

// Trac: https://trac.osgeo.org/geos/ticket/994
template<>
template<>
void object::test<11>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (MULTIPOLYGON (((1756184.5450235498137772 5080652.1657676175236702, 1756184.5450235498137772 5080614.4680011207237840, 1756123.0812665277626365 5080614.4680011207237840, 1756123.0812665277626365 5080430.0767300482839346, 1756061.6175095040816814 5080430.0767300482839346, 1756061.6175095040816814 5080491.5404870714992285, 1756000.1537524810992181 5080491.5404870714992285, 1756000.1537524810992181 5080653.7726632226258516, 1756184.5450235498137772 5080652.1657676175236702))), MULTIPOLYGON (((1755877.2262384353671223 5080654.8439269624650478, 1755877.2262384353671223 5080491.5404870714992285, 1756000.1537524810992181 5080491.5404870714992285, 1756000.1537524810992181 5080430.0767300482839346, 1756061.6175095040816814 5080430.0767300482839346, 1756061.6175095040816814 5080307.1492160055786371, 1756000.1537524810992181 5080307.1492160055786371, 1756000.1537524810992181 5080245.6854589805006981, 1756123.0812665277626365 5080245.6854589805006981, 1756123.0812665277626365 5080122.7579449350014329, 1756061.6175095040816814 5080122.7579449350014329, 1755938.6899954585824162 5080122.7579449350014329, 1755938.6899954585824162 5079938.3666738653555512, 1755877.2262384353671223 5079938.3666738653555512, 1755815.7624814116861671 5079938.3666738653555512, 1755815.7624814116861671 5079631.0478887520730495, 1756061.6175095040816814 5079631.0478887520730495, 1756061.6175095040816814 5079876.9029168421402574, 1756123.0812665277626365 5079876.9029168421402574, 1756123.0812665277626365 5080122.7579449350014329, 1756307.4725375962443650 5080122.7579449350014329, 1756307.4725375962443650 5080061.2941879099234939, 1756430.4000516426749527 5080061.2941879099234939, 1756430.4000516426749527 5079876.9029168421402574, 1756553.3275656879413873 5079876.9029168421402574, 1756553.3275656879413873 5079631.0478887520730495, 1756491.8638086654245853 5079631.0478887520730495, 1756491.8638086654245853 5079446.6566176833584905, 1756430.4000516426749527 5079446.6566176833584905, 1756246.0087805732619017 5079446.6566176833584905, 1756246.0087805732619017 5079200.8015895904973149, 1756184.5450235498137772 5079200.8015895904973149, 1756123.0812665277626365 5079200.8015895904973149, 1756123.0812665277626365 5079077.8740755440667272, 1756184.5450235498137772 5079077.8740755440667272, 1756184.5450235498137772 5079016.4103185208514333, 1756307.4725375962443650 5079016.4103185208514333, 1756307.4725375962443650 5078893.4828044744208455, 1756246.0087805732619017 5078893.4828044744208455, 1756246.0087805732619017 5078832.0190474493429065, 1756123.0812665277626365 5078832.0190474493429065, 1756123.0812665277626365 5078893.4828044744208455, 1756000.1537524810992181 5078893.4828044744208455, 1756000.1537524810992181 5078832.0190474493429065, 1755938.6899954585824162 5078832.0190474493429065, 1755877.2262384353671223 5078832.0190474493429065, 1755877.2262384353671223 5078709.0915334057062864, 1756000.1537524810992181 5078709.0915334057062864, 1756000.1537524810992181 5078705.9565899241715670, 1754455.4951298737432808 5078742.8940787250176072, 1754447.7975028993096203 5080667.3008222607895732, 1755877.2262384353671223 5080654.8439269624650478))))");
    ensure(nullptr != geom1_);

    geom2_ = GEOSUnaryUnion(geom1_);
    ensure(nullptr != geom2_);
}



} // namespace tut

