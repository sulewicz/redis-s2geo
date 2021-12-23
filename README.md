# Introduction

Simple Redis Module for geospatial queries, which utilizes S2 Region Coverer algorithm for indexing. Allows point with polygon and polygon with polygon overlap tests in constant time.

# How to build

## Build absl library (S2 dependency)
1. git clone https://github.com/abseil/abseil-cpp.git
1. cd abseil-cpp
1. mkdir build && cd build
1. cmake -DCMAKE_INSTALL_PREFIX=<ABSL_PATH> -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
1. make install

## Build redis-s2geo
1. mkdir build && cd build
1. cmake -DABSL_PATH="<ABSL_PATH>" ..
  1. On Mac you may have to provide -DOPENSSL_ROOT_DIR=`brew --prefix openssl`
1. make 

# How to use

TODO: In the meantime, see src/redistest/redistest.cc.
