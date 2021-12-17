#include "geo.h"
#include "parser.h"

#define REDISMODULE_API extern

extern "C"
{
#include "redismodule.h"
}

constexpr int kMaxCells = 128;

std::unique_ptr<S2Polygon> ParsePolygon(RedisModuleCtx *ctx, RedisModuleString *body)
{
    std::unique_ptr<S2Polygon> polygon(nullptr);
    size_t len;
    const char *cBody = RedisModule_StringPtrLen(body, &len);
    int ret = ParseS2Polygon(cBody, &polygon);
    if (ret != 0)
    {
        return nullptr;
    }
    return std::move(polygon);
}

std::unique_ptr<S2LatLng> ParseLatLng(RedisModuleCtx *ctx, RedisModuleString *body)
{
    std::unique_ptr<S2LatLng> latLng(nullptr);
    size_t len;
    const char *cBody = RedisModule_StringPtrLen(body, &len);
    int ret = ParseS2LatLng(cBody, &latLng);
    if (ret != 0)
    {
        return nullptr;
    }
    return std::move(latLng);
}

std::unordered_set<std::string> IndexPolygon(RedisModuleCtx *ctx, S2Polygon *polygon)
{
    std::unordered_set<std::string> ret;
    S2RegionCoverer::Options options;
    options.set_max_cells(kMaxCells);
    S2RegionCoverer coverer(options);
    S2CellUnion cellUnion = coverer.GetCovering(*polygon);
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        ret.insert(cellId.ToString());

        // indicates that the polygon occupies a subcell
        ret.insert(cellIdStr.substr(0, 1) + "*");
        for (int i = 3; i < cellIdStr.length(); i++) {
            ret.insert(cellIdStr.substr(0, i) + "*");
        }
        ret.insert(cellIdStr + "*");
    }
    return ret;
}

std::unordered_set<std::string> IndexPolygonForOverlapTest(RedisModuleCtx *ctx, S2Polygon *polygon)
{
    std::unordered_set<std::string> ret;
    S2RegionCoverer::Options options;
    options.set_max_cells(kMaxCells);
    S2RegionCoverer coverer(options);
    S2CellUnion cellUnion = coverer.GetCovering(*polygon);
    for (const S2CellId &cellId : cellUnion)
    {
        std::string cellIdStr = cellId.ToString();
        ret.insert(cellId.ToString()); // will result in polygons that occupies the same cell

        // will result in polygons that occupy (entirely) the parent cells
        ret.insert(cellIdStr.substr(0, 1));
        for (int i = 3; i < cellIdStr.length(); i++) {
            ret.insert(cellIdStr.substr(0, i));
        }
        // will result in polygons that occupy subcells
        ret.insert(cellIdStr + "*");
    }
    return ret;
}

std::unordered_set<std::string> IndexPoint(RedisModuleCtx *ctx, S2LatLng *latLng)
{
    std::unordered_set<std::string> ret;
    S2CellId cellId(latLng->ToPoint());
    std::string cellIdStr = cellId.ToString();
    ret.insert(cellIdStr.substr(0, 1));
    for (int i = 3; i < cellIdStr.length(); i++) {
        ret.insert(cellIdStr.substr(0, i));
    }
    ret.insert(cellIdStr);
    return ret;
}
