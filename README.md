# Introduction

Simple Redis Module for geospatial queries, which utilizes S2 Region Coverer algorithm for indexing ([demo](http://s2.sidewalklabs.com/regioncoverer/)). Allows point with polygon and polygon with polygon overlap tests in constant time.

<!-- toc -->
* [How to build](#how-to-build)
  * [Build absl library (S2 dependency)](#build-absl-library-s2-dependency)
  * [Build redis-s2geo](#build-redis-s2geo)
* [How to use](#how-to-use)
  * [Commands](#commands)
    * [S2GEO.ISET index](#s2geoiset-index)
    * [S2GEO.IGET index](#s2geoiget-index)
    * [S2GEO.IDEL index](#s2geoidel-index)
    * [S2GEO.POLYSET index polygonName polygonBody](#s2geopolyset-index-polygonname-polygonbody)
    * [S2GEO.POLYGET index polygonName](#s2geopolyget-index-polygonname)
    * [S2GEO.POLYMGET index polygonName [polygonName ...]](#s2geopolymget-index-polygonname-polygonname)
    * [S2GEO.POLYDEL index polygonName](#s2geopolydel-index-polygonname)
    * [S2GEO.POLYLIST index](#s2geopolylist-index)
    * [S2GEO.POLYSEARCH index polygonBody](#s2geopolysearch-index-polygonbody)
    * [S2GEO.POINTSEARCH index pointBody](#s2geopointsearch-index-pointbody)
    * [S2GEO.TEST (DEBUG)](#s2geotest-debug)
    * [S2GEO.POPULATE (DEBUG)](#s2geopopulate-debug)

<!-- toc stop -->

# How to build

## Build absl library (S2 dependency)
`git clone https://github.com/abseil/abseil-cpp.git`

`cd abseil-cpp`

`mkdir build && cd build`

`cmake -DCMAKE_INSTALL_PREFIX=<ABSL_PATH> -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..`

`make install`

## Build redis-s2geo
`mkdir build && cd build`

`cmake -DABSL_PATH="<ABSL_PATH>" .. # On Mac you may have to provide -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)`

`make`

# How to use

Add `loadmodule <BUILD_PATH>/libredis-s2geo.so` to your Redis config, or run `MODULE LOAD <BUILD_PATH>/libredis-s2geo.so` command on your Redis instance.

See a simple web app example here: [redis-s2geo-sample](https://github.com/sulewicz/redis-s2geo-sample).

## Commands

### S2GEO.ISET index
Creates a geometry index if one does not exist yet.

#### Return values
Simple string reply: OK if SET was executed correctly.

Error reply: Error if the index already exists.

#### Examples
```
redis> S2GEO.ISET index
OK

redis S2GEO.ISET index
(error) index already exists
```

### S2GEO.IGET index
Checks if index exists.

#### Return values
Simple string reply: OK if index exists.

Null reply: (nil) if the index does not exist or the provided key is not an index.

#### Examples
```
redis> S2GEO.IGET index
OK

redis> S2GEO.IGET nosuchindex
(nil)
```

### S2GEO.IDEL index
Deletes the provided index.

#### Return values
Integer reply: 1 if the index was deleted, 0 if the index does not exist or the provided key is not an index.

#### Examples
```
redis> S2GEO.IDEL index
(integer) 1

redis> S2GEO.IDEL index
(integer) 0
```

### S2GEO.POLYSET index polygonName polygonBody
Adds or updates a polygon in the index. The polygon body must be a three-dimensional array as in the "geometry" member field in a [GeoJSON polygon format](https://www.rfc-editor.org/rfc/rfc7946.html#section-3.1.6).

#### Return values
Integer reply: 1 if the polygon was added or updated successfully.

Error reply: Error if the index does not exist, the provided key is not an index, or the polygon format is invalid.

#### Examples
```
redis> S2GEO.POLYSET index polygon [[[-107.57812499999999,38.496593518947584],[-106.74316406249999,38.496593518947584],[-106.74316406249999,39.232253141714885],[-107.57812499999999,39.232253141714885],[-107.57812499999999,38.496593518947584]]]
(integer) 1

redis> S2GEO.POLYSET index polygon incorrectbody
(error) invalid polygon

redis> S2GEO.POLYSET nosuchindex polygon [[[-107.57812499999999,38.496593518947584],[-106.74316406249999,38.496593518947584],[-106.74316406249999,39.232253141714885],[-107.57812499999999,39.232253141714885],[-107.57812499999999,38.496593518947584]]]
(error) invalid index
```

### S2GEO.POLYGET index polygonName
Fetches polygon body from the index.

#### Return values
Simple string reply: Body of the polygon.

Null reply: (nil) if the polygon does not exist or the index does not exist.

Error reply: Error if the provided key is not an index, or the polygon name is invalid.

#### Examples
```
redis> S2GEO.POLYGET index polygon
[[[-107.57812499999999,38.496593518947584],[-106.74316406249999,38.496593518947584],[-106.74316406249999,39.232253141714885],[-107.57812499999999,39.232253141714885],[-107.57812499999999,38.496593518947584]]]

redis> S2GEO.POLYGET index nosuchpolygon
(nil)

redis S2GEO.POLYGET nosuchindex polygon
(nil)
```

### S2GEO.POLYMGET index polygonName [polygonName ...]
Fetches multiple polygon bodies from the index.

#### Return values
Array reply: list of polygon bodies associated with the given polygon names, in the same order as they are requested.

Error reply: Error if the provided key is not an index, or the polygon names are invalid.

#### Examples
```
redis> S2GEO.POLYMGET testindex blue red nosuchpolygon
1) "[[[-97.734375,36.77409249464195],[-94.10888671875,36.77409249464195],[-94.10888671875,41.29431726315258],[-97.734375,41.29431726315258],[-97.734375,36.77409249464195]]]"
2) "[[[-109.072265625,37.055177106660814],[-101.97509765625,37.055177106660814],[-101.97509765625,41.0130657870063],[-109.072265625,41.0130657870063],[-109.072265625,37.055177106660814]]]"
3) (nil)
redis> S2GEO.POLYMGET nosuchindex blue red nosuchpolygon
1) (nil)
2) (nil)
3) (nil)
```

### S2GEO.POLYDEL index polygonName
Deletes the provided polygon in the index.

#### Return values
Integer reply: 1 if the polygon was deleted successfully. 0 if the polygon does not exist, the index does not exist, or the provided key is not an index.

#### Examples
```
redis> S2GEO.POLYDEL index polygon
(integer) 1

redis> S2GEO.POLYDEL index nosuchpolygon
(integer) 0

redis> S2GEO.POLYDEL nosuchindex polygon
(integer) 0
```

### S2GEO.POLYLIST index
Lists the existing polygons in a given index.

#### Return values
Array reply: list of polygons in the index, or an empty list when index does not exist, or the provided key is not an index.

#### Examples
```
redis> S2GEO.POLYLIST index
1) "anotherPolygon"
2) "polygon"

redis> S2GEO.POLYLIST nosuchindex
(empty array)
```

### S2GEO.POLYSEARCH index polygonBody
Returns a list of polygons in a given index overlapping with the provided polygon.

Note: The list may contain false-positives for adjacent shapes due to the loss of precision when using S2 Region Coverer (TODO: provide more details in the implementation details section).

#### Return values
Array reply: list of polygons in the index overlapping with the provided polygon.

Error reply: Error if the index does not exist, the provided key is not an index, or the polygon format is invalid.

#### Examples
```
redis> S2GEO.POLYSEARCH index [[[-109.072265625,37.055177106660814],[-101.97509765625,37.055177106660814],[-101.97509765625,41.0130657870063],[-109.072265625,41.0130657870063],[-109.072265625,37.055177106660814]]]
1) "yellow"
2) "red"
3) "green"

redis> S2GEO.POLYSEARCH nosuchindex [[[-109.072265625,37.055177106660814],[-101.97509765625,37.055177106660814],[-101.97509765625,41.0130657870063],[-109.072265625,41.0130657870063],[-109.072265625,37.055177106660814]]]
(error) invalid index

redis> S2GEO.POLYSEARCH index incorrectbody
(error) invalid polygon
```

### S2GEO.POINTSEARCH index pointBody
Returns a list of polygons in a given index overlapping with the provided point. The point body must be a one-dimensional array as in the "geometry" member field in a [GeoJSON point format](https://www.rfc-editor.org/rfc/rfc7946.html#section-3.1.2).

Note: The list may contain false-positives for adjacent shapes due to the loss of precision when using S2 Region Coverer (TODO: provide more details in the implementation details section).

#### Return values
Array reply: list of polygons in the index overlapping with the provided point.

Error reply: Error if the index does not exist, the provided key is not an index, or the point format is invalid.

#### Examples
```
redis> S2GEO.POINTSEARCH index [-107.1826171875,38.92522904714054]
1) "yellow"
2) "red"

redis> S2GEO.POINTSEARCH nosuchindex [-107.1826171875,38.92522904714054]
(error) invalid index

redis> S2GEO.POINTSEARCH index incorrectbody
(error) invalid point
```

### S2GEO.TEST (DEBUG)
**Availaible in debug build only.**
Executes tests defined in src/redistest/redistest.cc

#### Return values
Simple string reply: OK if tests succeeded.

Error reply: Error if the test failed.

#### Examples
```
redis> S2GEO.TEST
"OK"
```

### S2GEO.POPULATE (DEBUG)
**Availaible in debug build only.**
Populates a testindex with sample shapes.

#### Return values
Simple string reply: OK if testindex was properly created and populated.

Error reply: Error if index creation failed.

#### Examples
```
redis> S2GEO.POPULATE
"OK"

redis> S2GEO.POLYLIST testindex
1) "blue"
2) "yellow"
3) "red"
4) "green"
```
