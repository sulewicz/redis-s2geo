#include "index.h"
#include <string.h>

#define REDISMODULE_API extern

extern "C"
{
#include "redismodule.h"
}

const char *INDEX_META_SUFFIX = "meta";
const char *INDEX_POLYGONS_SUFFIX = "polygons";
const char ENTITY_DELIM = ':';
const char *INDEX_PARAMS_KEY = "params";
const char *INDEX_PARAMS_VALUE = "<index>";

RedisModuleString *CreateIndexMetaHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s", cIndexName, ENTITY_DELIM, INDEX_META_SUFFIX);
}

RedisModuleString *CreateIndexPolygonsHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s", cIndexName, ENTITY_DELIM, INDEX_POLYGONS_SUFFIX);
}

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    RedisModuleString *metaObjectString = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGET", "sc", metaObjectString, INDEX_PARAMS_KEY);
    int type = RedisModule_CallReplyType(reply);
    if (type == REDISMODULE_REPLY_NULL)
    {
        return S2GEO_ERR_NO_SUCH_INDEX;
    }
    if (RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_STRING)
    {
        return S2GEO_ERR_INVALID_INDEX;
    }

    RedisModuleString *value = RedisModule_CreateStringFromCallReply(reply);
    size_t len;
    const char *cValue = RedisModule_StringPtrLen(value, &len);
    if (strcmp(INDEX_PARAMS_VALUE, cValue) != 0)
    {
        // TODO: what if somebody changes the value outside of this module?
        return S2GEO_ERR_INVALID_INDEX;
    }

    return 0; // all good
}

int ValidateEntityName(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    if (len == 0 || strchr(cIndexName, (int)ENTITY_DELIM) != 0)
    {
        return S2GEO_ERR_INVALID_ENTITY_NAME;
    }
    return 0;
}

int CreateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    RedisModuleString *metaObjectString = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HSET", "scc", metaObjectString, INDEX_PARAMS_KEY, INDEX_PARAMS_VALUE);
    // TODO: handle error here
    return 0;
}

int DeleteIndex(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    RedisModuleString *metaObjectString = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "DEL", "s", metaObjectString);
    // TODO: handle error here
    
    RedisModuleString *polygonsObjectString = CreateIndexPolygonsHashKey(ctx, indexName);
    reply = RedisModule_Call(ctx, "DEL", "s", polygonsObjectString);
    // TODO: handle error here

    // TODO: delete cells
    return 0;
}

int SetPolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString *polygonBody)
{
    RedisModuleString *polygonsObjectString = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HSET", "sss", polygonsObjectString, polygonName, polygonBody);
    // TODO: handle error here
    return 0;
}

int GetPolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString **output)
{
    RedisModuleString *polygonsObjectString = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGET", "ss", polygonsObjectString, polygonName);
    if (RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_STRING)
    {
        return S2GEO_ERR_NO_SUCH_POLYGON;
    }
    *output = RedisModule_CreateStringFromCallReply(reply);
    return 0;
}

int DeletePolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName)
{
    RedisModuleString *polygonsObjectString = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HDEL", "ss", polygonsObjectString, polygonName);
    // TODO: handle error here (e.g, poly does not exist)
    return 0;
}