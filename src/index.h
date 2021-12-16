#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include <string>

#define S2GEO_ERR_NO_SUCH_INDEX 1
#define S2GEO_ERR_INVALID_INDEX 2
#define S2GEO_ERR_INVALID_ENTITY_NAME 3

#define S2GEO_ERR_NO_SUCH_POLYGON 1

extern "C"
{
    typedef struct RedisModuleCtx RedisModuleCtx;
    typedef struct RedisModuleString RedisModuleString;
    typedef struct RedisModuleCallReply RedisModuleCallReply;
}

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateEntityName(RedisModuleCtx *ctx, RedisModuleString *indexName);

int CreateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int DeleteIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int SetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString *polygonBody);

int GetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString **output);

int DeletePolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName);

int SetPolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, const std::vector<std::string> &cells);

int DeletePolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName);

int GetPolygonsInCells(RedisModuleCtx *ctx, RedisModuleString *indexName, const std::vector<std::string> &cells, RedisModuleCallReply **polygons);

#endif // INDEX_H