cmake_minimum_required(VERSION 3.12)

project(redis-s2geo VERSION 0.0.2)

# If OpenSSL is installed in a non-standard location, configure with
# something like:
# OPENSSL_ROOT_DIR=/usr/local/opt/openssl cmake ..

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# No compiler-specific extensions, i.e. -std=c++17, not -std=gnu++17.
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Avoid "Policy CMP0042 is not set" warning on macOS.
if (APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
    SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif()

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

if(NOT ABSL_PATH) 
    message( FATAL_ERROR "Please provide -DABSL_PATH=<ABSL_PATH>" )
endif()

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${ABSL_PATH}")
include_directories("${ABSL_PATH}/include")

include_directories(src)

find_package(OpenSSL REQUIRED)
include_directories("${OPENSSL_INCLUDE_DIR}")

if(S2_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${S2_PATH}")
    include_directories("${OPENSSL_INCLUDE_DIR}" "${S2_PATH}/include")
    link_directories("${S2_PATH}/lib")
else()
    include(cmake/FetchGoogleTest.cmake)
    include_directories("${CMAKE_BINARY_DIR}/googletest/googletest/include")
    include(cmake/FetchS2.cmake)
endif()

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
    s2)

# Add version information to the target
set_target_properties(redis-s2geo PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

if(CMOCKA_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMOCKA_PATH}")
    include_directories("${CMOCKA_PATH}/include")
    link_directories("${CMOCKA_PATH}/lib")
else()
    include(cmake/FetchCMocka.cmake)
endif()

add_executable(redis-s2geo-test
    src/unittest/test_parser.cc
    src/unittest/test_geo.cc
    src/unittest/test.cc
    )

target_link_libraries(redis-s2geo-test PRIVATE cmocka-static redis-s2geo)

enable_testing()
add_test(NAME redis-s2geo-test COMMAND redis-s2geo-test)
