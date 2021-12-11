#ifndef INDEX_H
#define INDEX_H

#define S2GEO_ERR_NO_SUCH_INDEX 1
#define S2GEO_ERR_INVALID_INDEX 2
#define S2GEO_ERR_INVALID_INDEX_NAME 3

extern "C" {
    typedef struct RedisModuleCtx RedisModuleCtx;
    typedef struct RedisModuleString RedisModuleString;
}

RedisModuleString *CreateIndexMetaHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateIndexName(RedisModuleCtx *ctx, RedisModuleString *indexName);

int CreateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

#endif // INDEX_H