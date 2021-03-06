#ifndef INDEX_H
#define INDEX_H

#include <unordered_set>
#include <memory>
#include <string>

#define S2GEO_ERR_UNKNOWN 1
#define S2GEO_ERR_NO_SUCH_INDEX 2
#define S2GEO_ERR_INVALID_INDEX 3
#define S2GEO_ERR_INVALID_ENTITY_NAME 4
#define S2GEO_ERR_NO_SUCH_POLYGON 5

extern "C"
{
    typedef struct RedisModuleCtx RedisModuleCtx;
    typedef struct RedisModuleString RedisModuleString;
    typedef struct RedisModuleCallReply RedisModuleCallReply;
}

class S2Polygon;
class S2LatLng;

std::unique_ptr<S2Polygon> ParsePolygon(RedisModuleCtx *ctx, RedisModuleString *body);

std::unique_ptr<S2LatLng> ParseLatLng(RedisModuleCtx *ctx, RedisModuleString *body);

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateEntityName(RedisModuleCtx *ctx, RedisModuleString *indexName);

int CreateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int DeleteIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ListPolygons(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleCallReply **polygons);

int SetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString *polygonBody);

int GetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString **output);

int GetPolygonBodies(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString **polygonNames, size_t polygonCount, RedisModuleCallReply **output);

int DeletePolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName);

int SetPolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, const std::unordered_set<std::string> &cells);

int DeletePolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName);

int GetPolygonsInCells(RedisModuleCtx *ctx, RedisModuleString *indexName, const std::unordered_set<std::string> &cells, RedisModuleCallReply **polygons);

#endif // INDEX_H