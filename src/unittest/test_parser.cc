#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include "parser.h"

extern "C"
{
#include <cmocka.h>
}

void TestParsePolygon(void **state)
{
    std::unique_ptr<S2Polygon> polygon(nullptr);
    assert_int_equal(0, ParseS2Polygon("[[[31.9921875,46.31658418182218],[78.75,25.48295117535531],[82.6171875,64.16810689799152],[31.9921875,46.31658418182218]]]", &polygon));
    assert_int_equal(1, polygon->num_loops());
    assert_int_equal(3, polygon->loop(0)->num_vertices());

    assert_int_equal(0, ParseS2Polygon("[[[+31.9921875,+46.31658418182218],[+78.75,+25.48295117535531],[82.6171875,64.16810689799152],[31.9921875,46.31658418182218]]]", &polygon));
    assert_int_equal(1, polygon->num_loops());
    assert_int_equal(3, polygon->loop(0)->num_vertices());

    assert_int_equal(-90, ParseS2Polygon("[[[31.9921875,46.31658418182218],[78.75,25.48295117535531],[82.6171875,64.16810689799152]]]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-27, ParseS2Polygon("[[[1,1],[2,2],[3,3],[4,4]]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-59, ParseS2Polygon("[[[31.9921875,46.31658418182218],[78.75,25.48295117535531][82.6171875,64.16810689799152]]]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-1, ParseS2Polygon("]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(PARSE_ERR_LOOP_LIMIT, ParseS2Polygon("[[[31.9921875,46.31658418182218],[78.75,25.48295117535531],[82.6171875,64.16810689799152],[31.9921875,46.31658418182218]], [[31.9921875,46.31658418182218],[78.75,25.48295117535531],[82.6171875,64.16810689799152],[31.9921875,46.31658418182218]]]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-157, ParseS2Polygon("[[[31.9921875,46.31658418182218],[31.9921875,46.31658418182218],[31.9921875,46.31658418182218],[31.9921875,46.31658418182218],[31.9921875,46.31658418182218]]]", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-1, ParseS2Polygon("", &polygon));
    assert_null(polygon.get());

    assert_int_equal(-1, ParseS2Polygon(nullptr, &polygon));
    assert_null(polygon.get());

    assert_int_equal(-1, ParseS2Polygon("", nullptr));
    assert_null(polygon.get());
}

void TestParsePoint(void **state)
{
    std::unique_ptr<S2LatLng> latLng(nullptr);
    const float epsilon = 0.0000000001;
    assert_int_equal(0, ParseS2LatLng("[31.9921875,46.31658418182218]", &latLng));
    assert_float_equal(46.31658418182218, latLng->lat().degrees(), epsilon);
    assert_float_equal(31.9921875, latLng->lng().degrees(), epsilon);

    assert_int_equal(-30, ParseS2LatLng("[31.9921875,46.31658418182218", &latLng));
    assert_null(latLng.get());

    assert_int_equal(-14, ParseS2LatLng("[31.992187546.31658418182218]", &latLng));
    assert_null(latLng.get());

    assert_int_equal(-1, ParseS2LatLng("31.9921875,46.31658418182218]", &latLng));
    assert_null(latLng.get());

    assert_int_equal(-1, ParseS2LatLng("", &latLng));
    assert_null(latLng.get());

    assert_int_equal(-1, ParseS2LatLng(nullptr, &latLng));
    assert_null(latLng.get());

    assert_int_equal(-1, ParseS2LatLng("", nullptr));
    assert_null(latLng.get());
}