extern "C"
{
#include "redismodule.h"
}

#include "geo.h"
#include "index.h"

#ifdef DEBUG
int TestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
#endif

int SetIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);
    RedisModuleString *indexName = argv[1];

    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret != S2GEO_ERR_NO_SUCH_INDEX)
    {
        RedisModule_ReplyWithError(ctx, "index already exists");
        return REDISMODULE_ERR;
    }

    ret = CreateIndex(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "error while creating index");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    return REDISMODULE_OK;
}

int GetIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);
    RedisModuleString *indexName = argv[1];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret == S2GEO_ERR_NO_SUCH_INDEX)
    {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }
    if (ret == S2GEO_ERR_INVALID_INDEX)
    {
        RedisModule_ReplyWithError(ctx, "not an index");
        return REDISMODULE_ERR;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "unknown error");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    return REDISMODULE_OK;
}

int DeleteIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);
    RedisModuleString *indexName = argv[1];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret == S2GEO_ERR_NO_SUCH_INDEX)
    {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }
    if (ret == S2GEO_ERR_INVALID_INDEX)
    {
        RedisModule_ReplyWithError(ctx, "not an index");
        return REDISMODULE_ERR;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "unknown error during index validation");
        return REDISMODULE_ERR;
    }

    ret = DeleteIndex(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "failed to delete the index");
        return REDISMODULE_ERR;
    }
    RedisModule_ReplyWithLongLong(ctx, 1);

    return REDISMODULE_OK;
}

int SetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 4)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleString *indexName = argv[1];
    RedisModuleString *polygonName = argv[2];
    RedisModuleString *polygonBody = argv[3];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateEntityName(ctx, polygonName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index");
        return REDISMODULE_ERR;
    }

    std::unique_ptr<S2Polygon> polygon = ParsePolygon(ctx, polygonBody);
    if (polygon.get() == nullptr)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon");
        return REDISMODULE_ERR;
    }

    std::unordered_set<std::string> cells = IndexPolygon(polygon.get());
    if (cells.size() == 0)
    {
        RedisModule_ReplyWithError(ctx, "empty cell union for a given polygon");
        return REDISMODULE_ERR;
    }

    ret = SetPolygonBody(ctx, indexName, polygonName, polygonBody);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "error while storing polygon body");
        return REDISMODULE_ERR;
    }

    ret = DeletePolygonCells(ctx, indexName, polygonName);
    if (ret != 0 && ret != S2GEO_ERR_NO_SUCH_POLYGON)
    {
        RedisModule_ReplyWithError(ctx, "error while deleting polygon cells");
        return REDISMODULE_ERR;
    }

    ret = SetPolygonCells(ctx, indexName, polygonName, cells);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "error while storing polygon cells");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithLongLong(ctx, 1);
    return REDISMODULE_OK;
}

int GetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleString *indexName = argv[1];
    RedisModuleString *polygonName = argv[2];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateEntityName(ctx, polygonName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret == S2GEO_ERR_NO_SUCH_INDEX)
    {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index");
        return REDISMODULE_ERR;
    }

    RedisModuleString *polygonBody;
    ret = GetPolygonBody(ctx, indexName, polygonName, &polygonBody);
    if (ret == S2GEO_ERR_NO_SUCH_POLYGON) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon");
        return REDISMODULE_ERR;
    }
    RedisModule_ReplyWithString(ctx, polygonBody);

    return REDISMODULE_OK;
}

int DeletePolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleString *indexName = argv[1];
    RedisModuleString *polygonName = argv[2];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateEntityName(ctx, polygonName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret == S2GEO_ERR_NO_SUCH_INDEX)
    {
       RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index");
        return REDISMODULE_ERR;
    }

    ret = DeletePolygonBody(ctx, indexName, polygonName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "polygon deletion failed");
        return REDISMODULE_ERR;
    }

    ret = DeletePolygonCells(ctx, indexName, polygonName);
    if (ret == S2GEO_ERR_NO_SUCH_POLYGON) {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "polygon cells deletion failed");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithLongLong(ctx, 1);

    return REDISMODULE_OK;
}

int SearchPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleString *indexName = argv[1];
    RedisModuleString *polygonBody = argv[2];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index");
        return REDISMODULE_ERR;
    }

    std::unique_ptr<S2Polygon> polygon = ParsePolygon(ctx, polygonBody);
    if (polygon.get() == nullptr)
    {
        RedisModule_ReplyWithError(ctx, "invalid polygon");
        return REDISMODULE_ERR;
    }

    std::unordered_set<std::string> cells = IndexPolygonForOverlapTest(polygon.get());
    if (cells.size() == 0)
    {
        RedisModule_ReplyWithError(ctx, "empty cell union for a given polygon");
        return REDISMODULE_ERR;
    }

    RedisModuleCallReply *polygons;
    ret = GetPolygonsInCells(ctx, indexName, cells, &polygons);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "error while fetching polygons");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithCallReply(ctx, polygons);
    return REDISMODULE_OK;
}

int SearchPointCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    RedisModuleString *indexName = argv[1];
    RedisModuleString *pointBody = argv[2];
    int ret = ValidateEntityName(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index name");
        return REDISMODULE_ERR;
    }

    ret = ValidateIndex(ctx, indexName);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "invalid index");
        return REDISMODULE_ERR;
    }

    std::unique_ptr<S2LatLng> point = ParseLatLng(ctx, pointBody);
    if (point.get() == nullptr)
    {
        RedisModule_ReplyWithError(ctx, "invalid point");
        return REDISMODULE_ERR;
    }

    std::unordered_set<std::string> cells = IndexPoint(point.get());
    if (cells.size() == 0)
    {
        RedisModule_ReplyWithError(ctx, "no cells for a given point");
        return REDISMODULE_ERR;
    }

    RedisModuleCallReply *polygons;
    ret = GetPolygonsInCells(ctx, indexName, cells, &polygons);
    if (ret != 0)
    {
        RedisModule_ReplyWithError(ctx, "error while fetching polygons");
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithCallReply(ctx, polygons);
    return REDISMODULE_OK;
}

extern "C" int RedisModule_OnLoad(RedisModuleCtx *ctx)
{

    if (RedisModule_Init(ctx, "s2geo", 1, REDISMODULE_APIVER_1) ==
        REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.iset", SetIndexCommand, "write",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.iget", GetIndexCommand, "readonly",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.idel", DeleteIndexCommand, "write",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.polyset", SetPolygonCommand, "write",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.polyget", GetPolygonCommand, "readonly",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.polydel", DeletePolygonCommand, "write",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.polysearch", SearchPolygonCommand, "readonly",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, "s2geo.pointsearch", SearchPointCommand, "readonly",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

#ifdef DEBUG
    if (RedisModule_CreateCommand(ctx, "s2geo.test", TestCommand, "write",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }
#endif

    return REDISMODULE_OK;
}
