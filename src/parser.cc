#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <vector>
#include <s2/s2latlng.h>

#define LOOP_LIMIT 1
#define POINT_LIMIT 64

enum ParsingState
{
    POLYGON_START = 0,
    LOOP_START,
    POINT_START,
    POINT_LON,
    POINT_DELIM,
    POINT_LAT,
    POINT_END,
    LOOP_DELIM,
    POLYGON_DELIM,
    EOL,
};

int ParseS2LatLng(const char *body, std::unique_ptr<S2LatLng> *latLng)
{
    if (body == nullptr || latLng == nullptr) {
        return -1;
    }
    *latLng = nullptr;

    double latitude;
    double longitude;

    enum ParsingState state = POINT_START;
    int i = 0;
    while (body[i] != 0)
    {
        if (body[i] == ' ')
        {
            i++;
            continue;
        }
        switch (state)
        {
        case POINT_START:
            if (body[i] == '[')
            {
                state = POINT_LON;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_LON:
            if (isdigit(body[i]) || body[i] == '-' || body[i] == '+' || body[i] == '.')
            {
                char *end;
                longitude = strtod(body + i, &end);
                if (end != NULL && end > body + i)
                {
                    i = end - body;
                    state = POINT_DELIM;
                }
                else
                {
                    return -i - 1;
                }
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_DELIM:
            if (body[i] == ',')
            {
                state = POINT_LAT;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_LAT:
            if (isdigit(body[i]) || body[i] == '-' || body[i] == '+' || body[i] == '.')
            {
                char *end;
                latitude = strtod(body + i, &end);
                if (end != NULL && end > body + i)
                {
                    i = end - body;
                    state = POINT_END;
                }
                else
                {
                    return -i - 1;
                }
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_END:
            if (body[i] == ']')
            {
                state = EOL;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case EOL:
        default:
            return -i - 1;
            break;
        }
    }

    if (state != EOL)
    {
        return -i - 1;
    }

    *latLng = std::make_unique<S2LatLng>(std::move(S2LatLng::FromDegrees(latitude, longitude)));
    if (!(*latLng)->is_valid())
    {
        *latLng = nullptr;
        return -i - 1;
    }
    return 0;
}

int ParseS2Polygon(const char *body, std::unique_ptr<S2Polygon> *polygon)
{
    if (body == nullptr || polygon == nullptr) {
        return -1;
    }
    *polygon = nullptr;

    double latitude;
    double longitude;
    std::vector<S2Point> vertices;
    std::vector<std::unique_ptr<S2Loop>> loops;

    enum ParsingState state = POLYGON_START;
    int i = 0;
    int point_count = 0;
    int loop_count = 0;
    while (body[i] != 0)
    {
        if (body[i] == ' ')
        {
            i++;
            continue;
        }
        switch (state)
        {
        case POLYGON_START:
            if (body[i] == '[')
            {
                state = LOOP_START;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case LOOP_START:
            if (body[i] == '[')
            {
                if (loop_count >= LOOP_LIMIT)
                {
                    // Only one ring it supported for now
                    return PARSE_ERR_LOOP_LIMIT;
                }
                state = POINT_START;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_START:
            if (body[i] == '[')
            {
                if (point_count >= POINT_LIMIT)
                {
                    // Only one ring it supported for now
                    return POINT_LIMIT;
                }
                state = POINT_LON;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_LON:
            if (isdigit(body[i]) || body[i] == '-' || body[i] == '+' || body[i] == '.')
            {
                char *end;
                longitude = strtod(body + i, &end);
                if (end != NULL && end > body + i)
                {
                    i = end - body;
                    state = POINT_DELIM;
                }
                else
                {
                    return -i - 1;
                }
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_DELIM:
            if (body[i] == ',')
            {
                state = POINT_LAT;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_LAT:
            if (isdigit(body[i]) || body[i] == '-' || body[i] == '+' || body[i] == '.')
            {
                char *end;
                latitude = strtod(body + i, &end);
                if (end != NULL && end > body + i)
                {
                    i = end - body;
                    state = POINT_END;
                }
                else
                {
                    return -i - 1;
                }
            }
            else
            {
                return -i - 1;
            }
            break;
        case POINT_END:
            if (body[i] == ']')
            {
                point_count++;
                vertices.push_back(S2LatLng::FromDegrees(latitude, longitude).ToPoint());
                state = LOOP_DELIM;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case LOOP_DELIM:
            if (body[i] == ',')
            {
                state = POINT_START;
                i++;
            }
            else if (body[i] == ']')
            {
                if (vertices.size() < 4)
                {
                    return -i - 1;
                }

                if (vertices[0] == vertices[vertices.size() - 1])
                {
                    vertices.pop_back();
                }

                auto loop = std::make_unique<S2Loop>(vertices, S2Debug::DISABLE);
                if (!loop->IsValid())
                {
                    return -i - 1;
                }
                loops.push_back(std::move(loop));
                loop_count++;

                vertices.clear();
                point_count = 0;
                state = POLYGON_DELIM;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case POLYGON_DELIM:
            if (body[i] == ',')
            {
                state = LOOP_START;
                i++;
            }
            else if (body[i] == ']')
            {
                state = EOL;
                i++;
            }
            else
            {
                return -i - 1;
            }
            break;
        case EOL:
            return -i - 1;
        }
    }

    if (state != EOL)
    {
        return -i - 1;
    }

    if (loops.size() < 1)
    {
        return -i - 1;
    }

    *polygon = std::make_unique<S2Polygon>(std::move(loops), S2Debug::DISABLE);
    if (!(*polygon)->IsValid())
    {
        *polygon = nullptr;
        return -i - 1;
    }
    return 0;
}
