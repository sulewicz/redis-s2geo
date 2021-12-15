#include "index.h"
#include <string.h>

#define REDISMODULE_API extern

extern "C"
{
#include "redismodule.h"
}

const char *INDEX_META_SUFFIX = "meta";
const char *INDEX_POLYGONS_SUFFIX = "polygons";
const char *INDEX_CELLS_SUFFIX = "cells";
const char *POLYGON_CELLINFO_SUFFIX = "cellinfo";
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

RedisModuleString *CreateIndexCellsSetKey(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *cellId)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    const char *cCellId = RedisModule_StringPtrLen(cellId, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s%c%s", cIndexName, ENTITY_DELIM, INDEX_CELLS_SUFFIX, ENTITY_DELIM, cCellId);
}

RedisModuleString *CreatePolygonCellInfoSetKey(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    const char *cPolygonName = RedisModule_StringPtrLen(polygonName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s%c%s", cIndexName, ENTITY_DELIM, POLYGON_CELLINFO_SUFFIX, ENTITY_DELIM, cPolygonName);
}

RedisModuleString *CreatePolygonCellInfoPattern(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s%c*", cIndexName, ENTITY_DELIM, POLYGON_CELLINFO_SUFFIX, ENTITY_DELIM);
}

RedisModuleString *CreateIndexCellsPattern(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s%c*", cIndexName, ENTITY_DELIM, INDEX_CELLS_SUFFIX, ENTITY_DELIM);
}

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    RedisModuleString *metaHashKey = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGET", "sc", metaHashKey, INDEX_PARAMS_KEY);
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
    RedisModuleString *metaHashKey = CreateIndexMetaHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HSET", "scc", metaHashKey, INDEX_PARAMS_KEY, INDEX_PARAMS_VALUE);
    // TODO: handle error here
    return 0;
}

int DeleteIndex(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    RedisModuleString *polygonCellInfoPattern = CreatePolygonCellInfoPattern(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "KEYS", "s", polygonCellInfoPattern);
    // TODO: handle error here
    size_t len = RedisModule_CallReplyLength(reply);
    for (int idx = 0; idx < len; idx++)
    {
        RedisModuleString *key = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(reply, idx));
        // TODO: batch it up and move to scan
        RedisModule_Call(ctx, "DEL", "s", key);
        // TODO: handle error here
    }

    RedisModuleString *indexCellsPattern = CreateIndexCellsPattern(ctx, indexName);
    reply = RedisModule_Call(ctx, "KEYS", "s", indexCellsPattern);
    // TODO: handle error here
    len = RedisModule_CallReplyLength(reply);
    for (int idx = 0; idx < len; idx++)
    {
        size_t cLen;
        RedisModuleString *key = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(reply, idx));
        // TODO: batch it up and move to scan
        RedisModule_Call(ctx, "DEL", "s", key);
        // TODO: handle error here
    }

    RedisModuleString *polygonsHash = CreateIndexPolygonsHashKey(ctx, indexName);
    reply = RedisModule_Call(ctx, "DEL", "s", polygonsHash);
    // TODO: handle error here

    RedisModuleString *metaHashKey = CreateIndexMetaHashKey(ctx, indexName);
    reply = RedisModule_Call(ctx, "DEL", "s", metaHashKey);
    // TODO: handle error here

    return 0;
}

int SetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString *polygonBody)
{
    RedisModuleString *polygonsHash = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HSET", "sss", polygonsHash, polygonName, polygonBody);
    // TODO: handle error here
    return 0;
}

int GetPolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString **output)
{
    RedisModuleString *polygonsHash = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGET", "ss", polygonsHash, polygonName);
    if (RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_STRING)
    {
        return S2GEO_ERR_NO_SUCH_POLYGON;
    }
    *output = RedisModule_CreateStringFromCallReply(reply);
    return 0;
}

int DeletePolygonBody(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName)
{
    RedisModuleString *polygonsHash = CreateIndexPolygonsHashKey(ctx, indexName);
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "HDEL", "ss", polygonsHash, polygonName);
    // TODO: handle error here (e.g, poly does not exist)
    return 0;
}

int SetPolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, std::vector<std::string> cells)
{
    RedisModuleString *cellInfoSetKey = CreatePolygonCellInfoSetKey(ctx, indexName, polygonName);
    std::unique_ptr<RedisModuleString *[]> cellStrings
    { new RedisModuleString *[cells.size()] };

    for (int idx = 0; idx < cells.size(); idx++)
    {
        cellStrings.get()[idx] = RedisModule_CreateString(ctx, cells[idx].c_str(), cells[idx].length());
    }

    RedisModuleCallReply *reply = RedisModule_Call(ctx, "SADD", "sv", cellInfoSetKey, cellStrings.get(), cells.size());
    for (int idx = 0; idx < cells.size(); idx++)
    {
        RedisModuleString *indexCellsHashKey = CreateIndexCellsSetKey(ctx, indexName, cellStrings.get()[idx]);
        RedisModule_Call(ctx, "SADD", "ss", indexCellsHashKey, polygonName);
    }
    return 0;
}

int DeletePolygonCells(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName)
{
    RedisModuleString *cellInfoSetKey = CreatePolygonCellInfoSetKey(ctx, indexName, polygonName);
    RedisModuleCallReply *cells = RedisModule_Call(ctx, "SMEMBERS", "s", cellInfoSetKey);
    size_t len = RedisModule_CallReplyLength(cells);
    if (len == 0)
    {
        return S2GEO_ERR_NO_SUCH_POLYGON;
    }

    RedisModule_Call(ctx, "DEL", "s", cellInfoSetKey);

    for (int idx = 0; idx < len; idx++)
    {
        RedisModuleString *cellId = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(cells, idx));
        RedisModuleString *indexCellsHashKey = CreateIndexCellsSetKey(ctx, indexName, cellId);
        RedisModule_Call(ctx, "SREM", "ss", indexCellsHashKey, polygonName);
    }
    return 0;
}
