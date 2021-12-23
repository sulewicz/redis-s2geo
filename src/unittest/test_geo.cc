#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include "parser.h"
#include "geo.h"

extern "C"
{
#include <cmocka.h>
}

void AssertPointCellSetIncludesPoint(std::unordered_set<std::string> cells, S2LatLng *latLng)
{
    S2CellId cellId(latLng->ToPoint());
    cells = IndexPoint(latLng);
    for (const auto& cell : cells) {
        assert_string_equal(cellId.ToString().substr(0, cell.size()).c_str(), cell.c_str());
    }
}

void TestIndexPolygonForOverlapTest(void **state)
{
    std::unordered_set<std::string> cells;
    cells = IndexPolygonForOverlapTest(nullptr);
    assert_int_equal(0, cells.size());

    std::unique_ptr<S2Polygon> polygon(nullptr);
    assert_int_equal(0, ParseS2Polygon("[[[16.611328125,51.984880139916626],[18.6767578125,51.31688050404585],[20.390625,51.984880139916626],[18.610839843749996,52.93539665862318],[16.611328125,51.984880139916626]]]", &polygon));
    S2CellUnion cellUnion = GetPolygonCovering(polygon.get());
    assert_int_equal(95, cellUnion.size());
    cells = IndexPolygonForOverlapTest(polygon.get());
    assert_int_not_equal(0, cells.size());
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        cells.erase(cellId.ToString());

        // will result in polygons that occupy (entirely) the parent cells
        cells.erase(cellIdStr.substr(0, 1));
        for (int i = 3; i < cellIdStr.length(); i++)
        {
            cells.erase(cellIdStr.substr(0, i));
        }
        cells.erase(cellIdStr + "*");
    }
    assert_int_equal(0, cells.size());
}

void TestIndexPolygon(void **state)
{
    std::unordered_set<std::string> cells;
    cells = IndexPolygon(nullptr);
    assert_int_equal(0, cells.size());

    std::unique_ptr<S2Polygon> polygon(nullptr);
    assert_int_equal(0, ParseS2Polygon("[[[16.611328125,51.984880139916626],[18.6767578125,51.31688050404585],[20.390625,51.984880139916626],[18.610839843749996,52.93539665862318],[16.611328125,51.984880139916626]]]", &polygon));
    S2CellUnion cellUnion = GetPolygonCovering(polygon.get());
    assert_int_equal(95, cellUnion.size());
    cells = IndexPolygon(polygon.get());
    assert_int_not_equal(0, cells.size());
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        cells.erase(cellId.ToString());

        cells.erase(cellIdStr.substr(0, 1) + "*");
        for (int i = 3; i < cellIdStr.length(); i++)
        {
            cells.erase(cellIdStr.substr(0, i) + "*");
        }
        cells.erase(cellIdStr + "*");
    }
    assert_int_equal(0, cells.size());
}

void TestIndexPoint(void **state)
{
    std::unique_ptr<S2LatLng> latLng(nullptr);
    std::unordered_set<std::string> cells;
    cells = IndexPoint(nullptr);
    assert_int_equal(0, cells.size());

    assert_int_equal(0, ParseS2LatLng("[31.9921875,46.31658418182218]", &latLng));
    cells = IndexPoint(latLng.get());
    assert_int_equal(S2CellId::kMaxLevel + 1, cells.size());
    AssertPointCellSetIncludesPoint(cells, latLng.get());

    assert_int_equal(0, ParseS2LatLng("[0,0]", &latLng));
    cells = IndexPoint(latLng.get());
    assert_int_equal(S2CellId::kMaxLevel + 1, cells.size());
    AssertPointCellSetIncludesPoint(cells, latLng.get());

    assert_int_equal(0, ParseS2LatLng("[-60,-60]", &latLng));
    cells = IndexPoint(latLng.get());
    assert_int_equal(S2CellId::kMaxLevel + 1, cells.size());
    AssertPointCellSetIncludesPoint(cells, latLng.get());
}
