#ifndef GEO_H
#define GEO_H

#include <memory>
#include <unordered_set>
#include <s2/s2region_coverer.h>
#include <s2/s2polygon.h>

S2CellUnion GetPolygonCovering(S2Polygon *polygon);
std::unordered_set<std::string> IndexPolygonForOverlapTest(S2Polygon *polygon);
std::unordered_set<std::string> IndexPolygon(S2Polygon *polygon);
std::unordered_set<std::string> IndexPoint(S2LatLng *latLng);

#endif // GEO_H
