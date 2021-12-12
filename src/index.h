#ifndef INDEX_H
#define INDEX_H

#define S2GEO_ERR_NO_SUCH_INDEX 1
#define S2GEO_ERR_INVALID_INDEX 2
#define S2GEO_ERR_INVALID_ENTITY_NAME 3

#define S2GEO_ERR_NO_SUCH_POLYGON 1

extern "C" {
    typedef struct RedisModuleCtx RedisModuleCtx;
    typedef struct RedisModuleString RedisModuleString;
}

RedisModuleString *CreateIndexMetaHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName);
RedisModuleString *CreateIndexPolygonsHashKey(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int ValidateEntityName(RedisModuleCtx *ctx, RedisModuleString *indexName);

int CreateIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int DeleteIndex(RedisModuleCtx *ctx, RedisModuleString *indexName);

int SetPolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString *polygonBody);

int GetPolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName, RedisModuleString **output);

int DeletePolygon(RedisModuleCtx *ctx, RedisModuleString *indexName, RedisModuleString *polygonName);

#endif // INDEX_H