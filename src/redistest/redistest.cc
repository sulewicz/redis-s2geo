#ifdef DEBUG

#include <string.h>
#include <string>
#include <unordered_set>

#define REDISMODULE_API extern

extern "C"
{
#include "redismodule.h"
}

#define ERROR_BUFFER_SIZE 2048
char buffer[ERROR_BUFFER_SIZE];
size_t len;

#define _ERROR_INT(msg, expected, found, f, l)                                                                \
    snprintf(buffer, ERROR_BUFFER_SIZE, "%s (%s:%d): expected: '%d', got: '%d'", msg, f, l, expected, found); \
    fputs(buffer, stderr);
#define _ERROR_STR(msg, expected, found, f, l)                                                                \
    snprintf(buffer, ERROR_BUFFER_SIZE, "%s (%s:%d): expected: '%s', got: '%s'", msg, f, l, expected, found); \
    fputs(buffer, stderr);

#define ASSERT_INT_EQUAL(msg, expected, found)                \
    if (found != expected)                                    \
    {                                                         \
        _ERROR_INT(msg, expected, found, __FILE__, __LINE__); \
        RedisModule_ReplyWithError(ctx, buffer);              \
        return REDISMODULE_ERR;                               \
    }

#define ASSERT_SET_EQUAL(msg, expected, found)                                                          \
    if (found != expected)                                                                              \
    {                                                                                                   \
        _ERROR_STR(msg, SetToString(expected).c_str(), SetToString(found).c_str(), __FILE__, __LINE__); \
        RedisModule_ReplyWithError(ctx, buffer);                                                        \
        return REDISMODULE_ERR;                                                                         \
    }

#define ASSERT_REDIS_TYPE_EQUAL(msg, expected, found)                                    \
    if (RedisModule_CallReplyType(found) != expected)                                    \
    {                                                                                    \
        _ERROR_INT(msg, expected, RedisModule_CallReplyType(found), __FILE__, __LINE__); \
        RedisModule_ReplyWithError(ctx, buffer);                                         \
        return REDISMODULE_ERR;                                                          \
    }

#define ASSERT_REDIS_STRING_EQUAL(msg, expected, found)                     \
    {                                                                       \
        const char *foundStr = RedisModule_CallReplyStringPtr(found, &len); \
        if (strncmp(foundStr, expected, len) != 0)                          \
        {                                                                   \
            _ERROR_STR(msg, expected, foundStr, __FILE__, __LINE__);        \
            RedisModule_ReplyWithError(ctx, buffer);                        \
            return REDISMODULE_ERR;                                         \
        }                                                                   \
    }

const char *kTestIndexName = "testindex";

enum Polygons
{
    POLYGON_RED = 0,
    POLYGON_GREEN,
    POLYGON_BLUE,
    POLYGON_YELLOW,
    POLYGON_COUNT
};

const char *kPolygonBody[] = {
    "[[[-109.072265625,37.055177106660814],[-101.97509765625,37.055177106660814],[-101.97509765625,41.0130657870063],[-109.072265625,41.0130657870063],[-109.072265625,37.055177106660814]]]",
    "[[[-104.12841796875,38.151837403006766],[-99.66796875,38.151837403006766],[-99.66796875,39.67337039176558],[-104.12841796875,39.67337039176558],[-104.12841796875,38.151837403006766]]]",
    "[[[-97.734375,36.77409249464195],[-94.10888671875,36.77409249464195],[-94.10888671875,41.29431726315258],[-97.734375,41.29431726315258],[-97.734375,36.77409249464195]]]",
    "[[[-107.57812499999999,38.496593518947584],[-106.74316406249999,38.496593518947584],[-106.74316406249999,39.232253141714885],[-107.57812499999999,39.232253141714885],[-107.57812499999999,38.496593518947584]]]"};

const char *kPolygonName[] = {
    "red",
    "green",
    "blue",
    "yellow"};

enum Points
{
    POINT_CIRCLE = 0,
    POINT_TRIANGLE,
    POINT_SQUARE,
    POINT_COUNT
};

const char *kPointBody[] = {
    "[-99.8876953125,36.06686213257888]",
    "[-103.798828125,40.54720023441049]",
    "[-107.1826171875,38.92522904714054]"};

const char *kPointName[] = {
    "circle",
    "triangle",
    "square"};

std::string SetToString(std::unordered_set<std::string> set)
{
    std::string ret;
    for (const auto &element : set)
    {
        ret.append(element).append(", ");
    }
    return ret;
}

std::unordered_set<std::string> RedisArrayToSet(RedisModuleCallReply *array)
{
    std::unordered_set<std::string> ret;
    size_t len = RedisModule_CallReplyLength(array);
    for (size_t i = 0; i < len; i++)
    {
        size_t strLen;
        const char *str = RedisModule_CallReplyStringPtr(RedisModule_CallReplyArrayElement(array, i), &strLen);
        ret.insert(std::string(str, strLen));
    }
    return ret;
}

int TestIndex(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // Create the index
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not create the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not create the index", "OK", reply);

    // Try to overwrite the index
    reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("should not overwrite the index", REDISMODULE_REPLY_ERROR, reply);

    // Get the index
    reply = RedisModule_Call(ctx, "S2GEO.IGET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not fetch the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not fetch the index", "OK", reply);

    // Delete the index
    reply = RedisModule_Call(ctx, "S2GEO.IDEL", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not delete the index", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("could not delete the index", 1, (int)RedisModule_CallReplyInteger(reply));

    // Try to delete a non-existing index
    reply = RedisModule_Call(ctx, "S2GEO.IDEL", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("should not delete the index", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("should not delete the index", 0, (int)RedisModule_CallReplyInteger(reply));

    // Try to get a non-existing index
    reply = RedisModule_Call(ctx, "S2GEO.IGET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("should not fetch the index", REDISMODULE_REPLY_NULL, reply);

    return REDISMODULE_OK;
}

int TestPolygons(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // Create the index
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not create the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not create the index", "OK", reply);

    // List polygons (empty)
    reply = RedisModule_Call(ctx, "S2GEO.POLYLIST", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not list polygons", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("incorrect number of polygons returned", 0, (int)RedisModule_CallReplyLength(reply));

    // Create the polygon
    reply = RedisModule_Call(ctx, "S2GEO.POLYSET", "ccc", kTestIndexName, kPolygonName[POLYGON_RED], kPolygonBody[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("could not create the polygon", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("could not create the polygon", 1, (int)RedisModule_CallReplyInteger(reply));

    // List polygons (one polygon)
    reply = RedisModule_Call(ctx, "S2GEO.POLYLIST", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not list polygons", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("incorrect number of polygons returned", 1, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("incorrect polygons returned", std::unordered_set<std::string>({kPolygonName[POLYGON_RED]}), RedisArrayToSet(reply));

    // Get the polygon
    reply = RedisModule_Call(ctx, "S2GEO.POLYGET", "cc", kTestIndexName, kPolygonName[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("could not fetch the polygon", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not fetch the polygon", kPolygonBody[POLYGON_RED], reply);

    // Delete the polygon
    reply = RedisModule_Call(ctx, "S2GEO.POLYDEL", "cc", kTestIndexName, kPolygonName[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("could not delete the polygon", REDISMODULE_REPLY_INTEGER, reply);

    // Try to get a non-existing polygon
    reply = RedisModule_Call(ctx, "S2GEO.POLYGET", "cc", kTestIndexName, kPolygonName[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("should not fetch the polygon", REDISMODULE_REPLY_NULL, reply);

    // Try to delete a non-existing polygon
    reply = RedisModule_Call(ctx, "S2GEO.POLYDEL", "cc", kTestIndexName, kPolygonName[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("should not delete the polygon", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("should not delete the polygon", 0, (int)RedisModule_CallReplyInteger(reply));

    // Delete the index
    reply = RedisModule_Call(ctx, "S2GEO.IDEL", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not delete the index", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("could not delete the index", 1, (int)RedisModule_CallReplyInteger(reply));

    // Try to get a non-existing polygon from a non-existing index
    reply = RedisModule_Call(ctx, "S2GEO.POLYGET", "cc", kTestIndexName, kPolygonName[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("should not fetch the polygon", REDISMODULE_REPLY_NULL, reply);

    // Try to get list of polygons from a non-existing index (empty)
    reply = RedisModule_Call(ctx, "S2GEO.POLYLIST", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not list polygons", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("incorrect number of polygons returned", 0, (int)RedisModule_CallReplyLength(reply));

    return REDISMODULE_OK;
}

int TestPointSearch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // Create the index
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not create the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not create the index", "OK", reply);

    // Set up all the polygons, see test_data.png.
    for (size_t idx = 0; idx < POLYGON_COUNT; idx++)
    {
        reply = RedisModule_Call(ctx, "S2GEO.POLYSET", "ccc", kTestIndexName, kPolygonName[idx], kPolygonBody[idx]);
        ASSERT_REDIS_TYPE_EQUAL("could not create the polygon", REDISMODULE_REPLY_INTEGER, reply);
        ASSERT_INT_EQUAL("could not create the polygon", 1, (int)RedisModule_CallReplyInteger(reply));
    }

    // Should return no polygons
    reply = RedisModule_Call(ctx, "S2GEO.POINTSEARCH", "cc", kTestIndexName, kPointBody[POINT_CIRCLE]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform point search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong point search response", 0, (int)RedisModule_CallReplyLength(reply));

    // Should return the red polygon
    reply = RedisModule_Call(ctx, "S2GEO.POINTSEARCH", "cc", kTestIndexName, kPointBody[POINT_TRIANGLE]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform point search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong point search response", 1, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong point search response", std::unordered_set<std::string>({kPolygonName[POLYGON_RED]}), RedisArrayToSet(reply));

    // Should return the red and yellow polygons
    reply = RedisModule_Call(ctx, "S2GEO.POINTSEARCH", "cc", kTestIndexName, kPointBody[POINT_SQUARE]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform point search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong point search response", 2, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong point search response", std::unordered_set<std::string>({kPolygonName[POLYGON_RED], kPolygonName[POLYGON_YELLOW]}), RedisArrayToSet(reply));

    // Delete the index
    reply = RedisModule_Call(ctx, "S2GEO.IDEL", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not delete the index", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("could not delete the index", 1, (int)RedisModule_CallReplyInteger(reply));

    // Try searching in non-existing index
    reply = RedisModule_Call(ctx, "S2GEO.POINTSEARCH", "cc", kTestIndexName, kPointBody[POINT_SQUARE]);
    ASSERT_REDIS_TYPE_EQUAL("should not perform point search", REDISMODULE_REPLY_ERROR, reply);

    return REDISMODULE_OK;
}

int TestPolygonSearch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // Create the index
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not create the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not create the index", "OK", reply);

    // Set up all the polygons, see test_data.png.
    for (size_t idx = 0; idx < POLYGON_COUNT; idx++)
    {
        reply = RedisModule_Call(ctx, "S2GEO.POLYSET", "ccc", kTestIndexName, kPolygonName[idx], kPolygonBody[idx]);
        ASSERT_REDIS_TYPE_EQUAL("could not create the polygon", REDISMODULE_REPLY_INTEGER, reply);
        ASSERT_INT_EQUAL("could not create the polygon", 1, (int)RedisModule_CallReplyInteger(reply));
    }

    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_RED]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform polygon search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong polygon search response", 3, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong polygon search response", std::unordered_set<std::string>({kPolygonName[POLYGON_RED], kPolygonName[POLYGON_YELLOW], kPolygonName[POLYGON_GREEN]}), RedisArrayToSet(reply));

    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_GREEN]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform polygon search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong polygon search response", 2, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong polygon search response", std::unordered_set<std::string>({kPolygonName[POLYGON_GREEN], kPolygonName[POLYGON_RED]}), RedisArrayToSet(reply));

    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_BLUE]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform polygon search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong polygon search response", 1, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong polygon search response", std::unordered_set<std::string>({kPolygonName[POLYGON_BLUE]}), RedisArrayToSet(reply));

    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_YELLOW]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform polygon search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong polygon search response", 2, (int)RedisModule_CallReplyLength(reply));
    ASSERT_SET_EQUAL("wrong polygon search response", std::unordered_set<std::string>({kPolygonName[POLYGON_YELLOW], kPolygonName[POLYGON_RED]}), RedisArrayToSet(reply));

    // Delete the polygons
    for (size_t idx = 0; idx < POLYGON_COUNT; idx++)
    {
        reply = RedisModule_Call(ctx, "S2GEO.POLYDEL", "cc", kTestIndexName, kPolygonName[idx]);
        ASSERT_REDIS_TYPE_EQUAL("could not delete the polygon", REDISMODULE_REPLY_INTEGER, reply);
        ASSERT_INT_EQUAL("could not delete the polygon", 1, (int)RedisModule_CallReplyInteger(reply));
    }

    // Should return no polygons
    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_YELLOW]);
    ASSERT_REDIS_TYPE_EQUAL("could not perform polygon search", REDISMODULE_REPLY_ARRAY, reply);
    ASSERT_INT_EQUAL("wrong poly polygon response", 0, (int)RedisModule_CallReplyLength(reply));

    // Delete the index
    reply = RedisModule_Call(ctx, "S2GEO.IDEL", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not delete the index", REDISMODULE_REPLY_INTEGER, reply);
    ASSERT_INT_EQUAL("could not delete the index", 1, (int)RedisModule_CallReplyInteger(reply));

    // Try searching in non-existing index
    reply = RedisModule_Call(ctx, "S2GEO.POLYSEARCH", "cc", kTestIndexName, kPolygonBody[POLYGON_YELLOW]);
    ASSERT_REDIS_TYPE_EQUAL("should not perform polygon search", REDISMODULE_REPLY_ERROR, reply);

    return REDISMODULE_OK;
}

int TestCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);
    if (TestIndex(ctx, argv, argc) != REDISMODULE_OK)
    {
        return REDISMODULE_ERR;
    }
    if (TestPolygons(ctx, argv, argc) != REDISMODULE_OK)
    {
        return REDISMODULE_ERR;
    }
    if (TestPointSearch(ctx, argv, argc) != REDISMODULE_OK)
    {
        return REDISMODULE_ERR;
    }
    if (TestPolygonSearch(ctx, argv, argc) != REDISMODULE_OK)
    {
        return REDISMODULE_ERR;
    }

    RedisModule_ReplyWithCString(ctx, "OK");
    return REDISMODULE_OK;
}

int PopulateCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);
    // Create the index
    RedisModuleCallReply *reply = RedisModule_Call(ctx, "S2GEO.ISET", "c", kTestIndexName);
    ASSERT_REDIS_TYPE_EQUAL("could not create the index", REDISMODULE_REPLY_STRING, reply);
    ASSERT_REDIS_STRING_EQUAL("could not create the index", "OK", reply);

    // Set up all the polygons, see test_data.png.
    for (size_t idx = 0; idx < POLYGON_COUNT; idx++)
    {
        reply = RedisModule_Call(ctx, "S2GEO.POLYSET", "ccc", kTestIndexName, kPolygonName[idx], kPolygonBody[idx]);
        ASSERT_REDIS_TYPE_EQUAL("could not create the polygon", REDISMODULE_REPLY_INTEGER, reply);
        ASSERT_INT_EQUAL("could not create the polygon", 1, (int)RedisModule_CallReplyInteger(reply));
    }

    RedisModule_ReplyWithCString(ctx, "OK");
    return REDISMODULE_OK;
}

#endif
