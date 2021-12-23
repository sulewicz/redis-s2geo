cmake_minimum_required(VERSION 3.12)

project(redis-s2geo VERSION 0.10.0)

#include(cmake/FetchAbsl.cmake)
include(cmake/FetchS2.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Avoid "Policy CMP0042 is not set" warning on macOS.
if (APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# No compiler-specific extensions, i.e. -std=c++17, not -std=gnu++17.
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
# pthreads isn't used directly, but this is still required for std::thread.

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if (WIN32)
    # Use unsigned characters
    add_definitions(-J)
    # Make sure cmath header defines things like M_PI
    add_definitions(-D_USE_MATH_DEFINES)
    # Make sure Windows doesn't define min/max macros that interfere with STL
    add_definitions(-DNOMINMAX)
else()
    # Avoid megabytes of warnings like:
    # util/math/vector.h:178:16: warning: optimization attribute on
    # ‘double sqrt(double)’ follows definition but the attribute doesn’t
    # match [-Wattributes]
    add_definitions(-Wno-attributes)
    add_definitions(-Wno-deprecated-declarations)
endif()

# If OpenSSL is installed in a non-standard location, configure with
# something like:
# OPENSSL_ROOT_DIR=/usr/local/opt/openssl cmake ..

include_directories(src ${OPENSSL_INCLUDE_DIR})

add_library(redis-s2geo SHARED
            src/module.cc
            src/parser.cc
            src/geo.cc
            src/index.cc
            src/redistest/redistest.cc)

target_link_libraries(
    redis-s2geo
    ${OPENSSL_LIBRARIES}
    c
    s2
    s2testing)

# Add version information to the target
set_target_properties(redis-s2geo PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

include(cmake/FetchCMocka.cmake)

add_executable(redis-s2geo-test
    src/unittest/test_parser.cc
    src/unittest/test_geo.cc
    src/unittest/test.cc
    )

target_link_libraries(redis-s2geo-test PRIVATE cmocka-static redis-s2geo)

enable_testing()
add_test(NAME redis-s2geo-test COMMAND redis-s2geo-test)
