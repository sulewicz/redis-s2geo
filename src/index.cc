#include "index.h"
#include <string.h>

#define REDISMODULE_API extern

extern "C" {
#include "redismodule.h"
}

const char *INDEX_META_SUFFIX = "meta";
const char INDEX_DELIM = ':';
const char *INDEX_PARAMS_KEY = "params";
const char *INDEX_PARAMS_VALUE = "<index>";

RedisModuleString *CreateIndexMetaHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    return RedisModule_CreateStringPrintf(ctx, "%s%c%s", cIndexName, INDEX_DELIM, INDEX_META_SUFFIX);
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

int ValidateIndexName(RedisModuleCtx *ctx, RedisModuleString *indexName)
{
    size_t len;
    const char *cIndexName = RedisModule_StringPtrLen(indexName, &len);
    if (len == 0 || strchr(cIndexName, (int)INDEX_DELIM) != 0)
    {
        return S2GEO_ERR_INVALID_INDEX_NAME;
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