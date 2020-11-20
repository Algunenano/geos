//
// Test Suite for C-API GEOSClipByRect

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosclipbyrect_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSWKTWriter* w_;

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

    void
    isEqual(GEOSGeom g, const char* exp_wkt)
    {
        geom3_ = GEOSGeomFromWKT(exp_wkt);
        bool eq = GEOSEquals(geom3_, g) != 0;
        if(! eq) {
            std::printf("EXP: %s\n", exp_wkt);
            char* obt_wkt = GEOSWKTWriter_write(w_, g);
            std::printf("OBT: %s\n", obt_wkt);
            free(obt_wkt);
        }
        ensure(eq);
    }

    test_capigeosclipbyrect_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setOutputDimension(w_, 3);
        GEOSWKTWriter_setTrim(w_, 1);
        GEOSWKTWriter_setRoundingPrecision(w_, 8);
    }

    ~test_capigeosclipbyrect_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        GEOSWKTWriter_destroy(w_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeosclipbyrect_data> group;
typedef group::object object;

group test_capigeosclipbyrect_group("capi::GEOSClipByRect");

//
// Test Cases
//

/// Point outside
template<> template<> void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(0 0)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT EMPTY");
}

/// Point inside
template<> template<> void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(15 15)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT(15 15)");
}

/// Point on boundary
template<> template<> void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(15 10)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT EMPTY");
}

/// Line outside
template<> template<> void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, -5 5)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING EMPTY");
}

/// Line inside
template<> template<> void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(15 15, 16 15)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING(15 15, 16 15)");
}

/// Line on boundary
template<> template<> void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(10 15, 10 10, 15 10)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING (10 15, 10 10, 15 10)");
}

/// Line splitting rectangle
template<> template<> void object::test<7>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(10 5, 25 20)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING (15 10, 20 15)");
}

/// Polygon shell (CCW) fully on rectangle boundary
template<> template<> void object::test<8>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
}

/// Polygon shell (CW) fully on rectangle boundary
template<> template<> void object::test<9>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((10 10, 10 20, 20 20, 20 10, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
}

/// Polygon hole (CCW) fully on rectangle boundary
template<> template<> void object::test<10>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 20 10, 20 20, 10 20, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON EMPTY");
}

/// Polygon hole (CW) fully on rectangle boundary
template<> template<> void object::test<11>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 10 20, 20 20, 20 10, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON EMPTY");
}

/// Polygon fully within rectangle
template<> template<> void object::test<12>
()
{
    const char* wkt = "POLYGON((1 1, 1 30, 30 30, 30 1, 1 1),(10 10, 20 10, 20 20, 10 20, 10 10))";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 0, 0, 40, 40);
    isEqual(geom2_, wkt);
}

/// Polygon overlapping rectangle
template<> template<> void object::test<13>
()
{
    const char* wkt = "POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 20 10, 20 20, 10 20, 10 10))";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 5, 5, 15, 15);
    isEqual(geom2_, "POLYGON ((5 5, 5 15, 10 15, 10 10, 15 10, 15 5, 5 5))");
}

/// Trac: https://trac.osgeo.org/geos/ticket/1056
template<> template<> void object::test<14>
()
{
    const char* wkt = "LINESTRING(0 0, 0 15, 15 15, 1 1)";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 0, 0, 10, 10);
    /*
     * EXP: MULTILINESTRING((0 0,0 10),(10 10,1 1))
     * OBT: LINESTRING (10 10, 1 1)
     */
    isEqual(geom2_, "MULTILINESTRING((0 0,0 10),(10 10,1 1))");
}

template<> template<> void object::test<15>
()
{
    const char* wkt = "LINESTRING(-1 -1, 1 0, 2 0, -1 -1)";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 0, 0, 10, 10);
    /*
     * EXP: MULTILINESTRING((0 0,0 10),(10 10,1 1))
     * OBT: LINESTRING (10 10, 1 1)
     */
    isEqual(geom2_, "LINESTRING(1 0, 2 0)");
}

template<> template<> void object::test<16>
()
{
    const char* wkt = "LINESTRING(0 0, 0 10, 10 10, 10 0, 0 0)";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 0, 0, 10, 10);
    /*
     * EXP: MULTILINESTRING((0 0,0 10),(10 10,1 1))
     * OBT: LINESTRING (10 10, 1 1)
     */
    isEqual(geom2_, "LINESTRING(0 0, 0 10, 10 10, 10 0, 0 0)");
}

/// Comes from https://trac.osgeo.org/geos/ticket/1056
template<> template<> void object::test<17>
()
{
    const char* wkt = "LINESTRING(2542 4287 5,2540 4299 5.361999999965,2537 4305 5.910000000033,2530 4311 6.832999999984,2519 4314 8.670000000042,2511 4318 10.351999999955,2504 4325 12.388000000035,2499 4331 14.146999999997,2497 4337 15.354999999981,2497 4363 18.652000000002,2484 4382 19.369999999995,2481 4390 19.572999999975,2480 4394 19.675999999978,2485 4406 20,2484 4410 20,2480 4415 20.123999999952,2466 4423 20.464999999967,2464 4425 20.503999999957,2458 4438 21.265000000014,2453 4443 21.55700000003,2451 4444 21.589000000036,2446 4446 21.898000000045,2441 4450 22.373000000021,2438 4455 22.876000000047,2437 4460 23.35699999996,2435 4475 24.327999999979,2432 4479 24.689999999944,2428 4483 24.974999999977,2423 4484 25.236000000033,2419 4483 25.587000000058,2414 4481 25.886000000057,2408 4480 26.34600000002,2404 4483 26.780999999959,2401 4488 27.089999999967,2398 4491 27.447000000044,2392 4493 28.158999999985,2385 4494 28.90399999998,2379 4492 29.648000000045,2375 4489 30.082999999984,2370 4481 30.947000000044,2368 4478 31.459999999963,2340 4479 34.521999999997,2320 4483 38.736999999965,2294 4491 42,2289 4493 42,2283 4498 42.108000000008,2277 4504 42.302000000025,2269 4517 43.079000000027,2263 4522 43.53899999999,2261 4523 43.706000000006,2255 4523 44.201000000001,2243 4521 45.5,2239 4517 45.886000000057,2234 4502 47.400999999954,2213 4462 49.908999999985,2211 4454 50.160999999964,2210 4443 50.631999999983,2211 4429 51.402000000002,2214 4418 52.089000000036,2212 4414 52.373000000021,2199 4404 53.283999999985,2188 4397 54,2184 4397 54.295000000042,2179 4398 54.626999999979,2166 4405 55,2159 4405 55,2155 4402 55.070999999996,2153 4390 55.351000000024,2149 4381 55.976000000024,2138 4334 60.5,2136 4330 60.895000000019,2131 4324 61.405999999959,2121 4315 62.069999999949,2103 4290 63.123999999952,2097 4284 63.302999999956,2090 4279 63.515999999945,2085 4273 63.805000000051,2076 4266 64.10699999996,2073 4262 64.185000000056,2072 4260 64.197999999975,2072 4253 64.396999999997,2072 4242 64.701000000001,2074 4228 65,2075 4208 65,2071 4202 65,2064 4197 65,2061 4193 65,2061 4186 65,2063 4179 65,2064 4169 65,2060 4154 65,2056 4145 65.222999999998,2052 4140 65.363999999943,2047 4139 65.493000000017,2039 4140 65.812000000035,2023 4146 66,2021 4146 66,2013 4140 66,2011 4139 66,2009 4140 66,2005 4143 66,2003 4143 66,2001 4142 66,1990 4130 66,1986 4124 66,1984 4118 66)";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, -256,256,4352,4352);

    for (int i = 0; i < GEOSGetNumGeometries(geom2_); i++)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom2_, i);
        bool hasz = GEOSHasZ(g);
        if (!hasz) {
            std::printf("Expected subgeom %d to have z-coordinate and it doesn't", i);
        }
        ensure(hasz);
    }
    ensure(GEOSHasZ(geom2_) == true);
    isEqual(geom2_, "MULTILINESTRING Z ((2542 4287 5, 2540 4299 5.3619999999649996525, 2537 4305 5.9100000000330004113, 2530 4311 6.8329999999839996505, 2519 4314 8.6700000000420001101, 2511 4318 10.351999999954999865, 2504 4325 12.388000000034999459, 2499 4331 14.146999999996999975, 2497 4337 15.354999999981000514, 2497 4352 17.257115384608500364), (2142.2127659574466634 4352 58.76740425532834422, 2138 4334 60.5, 2136 4330 60.895000000019003039, 2131 4324 61.40599999995900049, 2121 4315 62.069999999948997527, 2103 4290 63.123999999951998063, 2097 4284 63.302999999956000465, 2090 4279 63.51599999994500223, 2085 4273 63.805000000051002473, 2076 4266 64.106999999959995762, 2073 4262 64.185000000055993041, 2072 4260 64.197999999974996399, 2072 4253 64.39699999999700708, 2072 4242 64.701000000001002377, 2074 4228 65, 2075 4208 65, 2071 4202 65, 2064 4197 65, 2061 4193 65, 2061 4186 65, 2063 4179 65, 2064 4169 65, 2060 4154 65, 2056 4145 65.222999999997995246, 2052 4140 65.363999999943004582, 2047 4139 65.493000000017005391, 2039 4140 65.812000000034998948, 2023 4146 66, 2021 4146 66, 2013 4140 66, 2011 4139 66, 2009 4140 66, 2005 4143 66, 2003 4143 66, 2001 4142 66, 1990 4130 66, 1986 4124 66, 1984 4118 66))");
}

} // namespace tut