#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

extern "C"
{
#include <cmocka.h>
}

void TestParsePolygon(void **state);
void TestParsePoint(void **state);

void TestIndexPolygonForOverlapTest(void **state);
void TestIndexPolygon(void **state);
void TestIndexPoint(void **state);

int main()
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(TestParsePolygon),
            cmocka_unit_test(TestParsePoint),
            cmocka_unit_test(TestIndexPolygonForOverlapTest),
            cmocka_unit_test(TestIndexPolygon),
            cmocka_unit_test(TestIndexPoint)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
