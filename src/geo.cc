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
        if (ret < 0)
        {
            RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Invalid polygon provided, error at index %d", (-ret) - 1);
            RedisModule_ReplyWithError(ctx, "format error in polygon body");
        }
        else
        {
            RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Invalid polygon provided, err=%d", ret);
            RedisModule_ReplyWithError(ctx, "invalid polygon");
        }
        return nullptr;
    }
    return std::move(polygon);
}

std::vector<std::string> IndexPolygon(RedisModuleCtx *ctx, S2Polygon *polygon)
{
    std::vector<std::string> ret;
    S2RegionCoverer coverer;
    S2CellUnion cellUnion = coverer.GetCovering(*polygon);
    RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Cell count=%zu", cellUnion.size());
    for (const S2CellId &cellId : cellUnion)
    {
        ret.push_back(cellId.ToString());
        RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Cell %s", cellId.ToString().c_str());
    }
    return ret;
}
