extern "C"
{
#include "../redismodule.h"
}

#include <parser.h>
#include <s2/s2region_coverer.h>
#include <s2/s2latlng.h>
#include <s2/s2latlng_rect.h>

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
  RedisModule_ReplyWithError(ctx, "SetIndexCommand: implement");
  // TODO: HSET <INDEX>.meta params

  return REDISMODULE_ERR;
}

int GetIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  RedisModule_ReplyWithError(ctx, "GetIndexCommand: implement");
  // TODO: HGET <INDEX>.meta params

  return REDISMODULE_ERR;
}

int DeleteIndexCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  RedisModule_ReplyWithError(ctx, "DeleteIndexCommand: implement");
  // TODO: DEL <INDEX>.meta
  // TODO: DEL <INDEX>.polygons
  // TODO: DEL <INDEX>.cells

  return REDISMODULE_ERR;
}

int SetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  if (argc != 4) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);
  std::unique_ptr<S2Polygon> polygon(nullptr);
  size_t l1;
  const char *body = RedisModule_StringPtrLen(argv[3], &l1);
  int ret = ParseS2Polygon(body, &polygon);
  if (ret != 0) {
    if (ret < 0) {
      RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Invalid polygon provided, error at index %d", (-ret) - 1);
      RedisModule_ReplyWithError(ctx, "format error in polygon body");
    } else {
      RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Invalid polygon provided, err=%d", ret);
      RedisModule_ReplyWithError(ctx, "invalid polygon");
    }
    return REDISMODULE_ERR;
  }

  S2RegionCoverer coverer;
  S2CellUnion cellUnion = coverer.GetCovering(*polygon);
  RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Cell count=%zu", cellUnion.size());
  for (const S2CellId &cellId : cellUnion) {
    RedisModule_Log(ctx, REDISMODULE_LOGLEVEL_WARNING, "Cell %s", cellId.ToString().c_str());
  }
  RedisModule_ReplyWithNull(ctx);

  // TODO: HSET <INDEX>.polygons <NAME> <BODY>
  // TODO: index polygon in <INDEX>.cells

  return REDISMODULE_OK;
}

int GetPolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  RedisModule_ReplyWithError(ctx, "GetPolygonCommand: implement");

  // TODO: HGET <INDEX>.polygons <NAME>

  return REDISMODULE_ERR;
}

int DeletePolygonCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  RedisModule_ReplyWithError(ctx, "DeletePolygonCommand: implement");
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
