#include "geo.h"

constexpr int kMaxCells = 128;

S2CellUnion GetPolygonCovering(S2Polygon *polygon)
{
    S2CellUnion ret;
    if (polygon == nullptr)
    {
        return ret;
    }
    S2RegionCoverer::Options options;
    options.set_max_cells(kMaxCells);
    S2RegionCoverer coverer(options);
    ret = coverer.GetCovering(*polygon);
    return ret;
}

std::unordered_set<std::string> IndexPolygon(S2Polygon *polygon)
{
    std::unordered_set<std::string> ret;
    if (polygon == nullptr)
    {
        return ret;
    }
    S2CellUnion cellUnion = GetPolygonCovering(polygon);
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        ret.insert(cellId.ToString());

        // indicates that the polygon occupies a subcell
        ret.insert(cellIdStr.substr(0, 1) + "*");
        for (int i = 3; i < cellIdStr.length(); i++)
        {
            ret.insert(cellIdStr.substr(0, i) + "*");
        }
        ret.insert(cellIdStr + "*");
    }
    return ret;
}

std::unordered_set<std::string> IndexPolygonForOverlapTest(S2Polygon *polygon)
{
    std::unordered_set<std::string> ret;
    if (polygon == nullptr)
    {
        return ret;
    }
    S2CellUnion cellUnion = GetPolygonCovering(polygon);
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        ret.insert(cellId.ToString()); // will result in polygons that occupies the same cell

        // will result in polygons that occupy (entirely) the parent cells
        ret.insert(cellIdStr.substr(0, 1));
        for (int i = 3; i < cellIdStr.length(); i++)
        {
            ret.insert(cellIdStr.substr(0, i));
        }
        // will result in polygons that occupy subcells
        ret.insert(cellIdStr + "*");
    }
    return ret;
}

std::unordered_set<std::string> IndexPoint(S2LatLng *latLng)
{
    std::unordered_set<std::string> ret;
    if (latLng == nullptr)
    {
        return ret;
    }
    S2CellId cellId(latLng->ToPoint());
    std::string cellIdStr = cellId.ToString();
    ret.insert(cellIdStr.substr(0, 1));
    for (int i = 3; i < cellIdStr.length(); i++)
    {
        ret.insert(cellIdStr.substr(0, i));
    }
    ret.insert(cellIdStr);
    return ret;
}
