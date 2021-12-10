#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <s2/s2polygon.h>

#define PARSE_ERR_LOOP_LIMIT 1
#define PARSE_ERR_POINT_LIMIT 2

int ParseS2Polygon(const char *body, std::unique_ptr<S2Polygon> *polygon);

#endif // PARSER_H