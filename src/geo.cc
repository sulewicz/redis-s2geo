#include "geo.h"
#include "parser.h"

#define REDISMODULE_API extern

extern "C"
{
#include "redismodule.h"
}

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

std::vector<std::string> IndexPolygon(RedisModuleCtx *ctx, S2Polygon *polygon)
{
    std::vector<std::string> ret;
    S2RegionCoverer coverer;
    S2CellUnion cellUnion = coverer.GetCovering(*polygon);
    for (const S2CellId &cellId : cellUnion)
    {
        ret.push_back(cellId.ToString());
    }
    return ret;
}
