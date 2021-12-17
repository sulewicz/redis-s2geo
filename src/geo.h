#ifndef GEO_H
#define GEO_H

#include <memory>
#include <unordered_set>
#include <s2/s2region_coverer.h>
#include <s2/s2polygon.h>

extern "C"
{
    typedef struct RedisModuleCtx RedisModuleCtx;
    typedef struct RedisModuleString RedisModuleString;
}

std::unique_ptr<S2Polygon> ParsePolygon(RedisModuleCtx *ctx, RedisModuleString *body);

std::unique_ptr<S2LatLng> ParseLatLng(RedisModuleCtx *ctx, RedisModuleString *body);

std::unordered_set<std::string> IndexPolygonForOverlapTest(RedisModuleCtx *ctx, S2Polygon *polygon);

std::unordered_set<std::string> IndexPolygon(RedisModuleCtx *ctx, S2Polygon *polygon);

std::unordered_set<std::string> IndexPoint(RedisModuleCtx *ctx, S2LatLng *latLng);

#endif // GEO_H
