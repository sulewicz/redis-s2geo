#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include "parser.h"

extern "C"
{
#include <cmocka.h>
}

static void test_parse_polygon(void **state)
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
}

int main()
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(test_parse_polygon),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
