extern "C"
{
#include "redismodule.h"
}

#include "geo.h"
#include "index.h"

/*
# Data layout:

## Meta data:
Hash:
<INDEX>.meta:
- params -> params (TODO)

## Shape data:
Hash:
<INDEX>.polygons:
- <NAME> -> <BODY>

## Cell data:
Set:
<INDEX>.<CELLID> -> [<NAME>]
*/

// Unit test entry point for the module
int TestModule(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);
    S2RegionCoverer coverer;
    auto region = S2LatLngRect::FromPointPair(S2LatLng::FromDegrees(45.583289756006316, -109.6875), S2LatLng::FromDegrees(-103.71093749999999, 49.15296965617042));
    auto result = coverer.GetCovering(region);
    auto str = result.ToString();
    RedisModule_ReplyWithCString(ctx, str.c_str());

    return REDISMODULE_OK;
}

int SetIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);
    RedisModuleString *indexName = argv[1];

    int ret = ValidateIndexName(ctx, indexName);
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

    RedisModule_ReplyWithLongLong(ctx, 1); // TODO: return something meaningful

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
    int ret = ValidateIndexName(ctx, indexName);
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

    RedisModule_ReplyWithLongLong(ctx, 1); // TODO: return something meaningful

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
    int ret = ValidateIndexName(ctx, indexName);
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

    // TODO: DEL <INDEX>.meta
    // TODO: DEL <INDEX>.polygons
    // TODO: DEL <INDEX>.cells

    RedisModuleString *metaObjectString = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "DEL", "s", metaObjectString);
    RedisModule_ReplyWithCallReply(ctx, reply); // TODO: change this once we handle the rest of the attributes

    return REDISMODULE_OK;
}

int SetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 4)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // TODO: verify if index exists
    // TODO: validate polygon name

    std::unique_ptr<S2Polygon> polygon = ParsePolygon(ctx, argv[3]);
    if (polygon.get() == nullptr)
    {
        return REDISMODULE_ERR;
    }

    S2CellUnion cellUnion = IndexPolygon(ctx, polygon.get());
    if (cellUnion.size() == 0)
    {
        return REDISMODULE_ERR;
    }

    // TODO: HSET <INDEX>.polygons <NAME> <BODY>
    // TODO: index polygon in <INDEX>.cells
    RedisModule_ReplyWithNull(ctx);
    return REDISMODULE_OK;
}

int GetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_ReplyWithError(ctx, "GetPolygonCommand: implement");
    // TODO: verify if index exists in a streamlined way
    // TODO: HGET <INDEX>.polygons <NAME>

    return REDISMODULE_ERR;
}

int DeletePolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_ReplyWithError(ctx, "DeletePolygonCommand: implement");
    // TODO: verify if index exists in a streamlined way
    // TODO: HDEL <INDEX>.polygons <NAME>
    // TODO: remove polygon from <INDEX>.cells

    return REDISMODULE_ERR;
}

int SearchPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_ReplyWithError(ctx, "SearchPolygonCommand: implement");

    // TODO: Generate cells and query <INDEX>.cells and return names

    return REDISMODULE_ERR;
}

int SearchPointCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_ReplyWithError(ctx, "SearchPointCommand: implement");

    // TODO: Generate cell and query <INDEX>.cells and return names

    return REDISMODULE_ERR;
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

    if (RedisModule_CreateCommand(ctx, "s2geo.polydel", GetPolygonCommand, "write",
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

    if (RedisModule_CreateCommand(ctx, "s2geo.test", TestModule, "readonly",
                                  1, 1, 1) == REDISMODULE_ERR)
    {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
