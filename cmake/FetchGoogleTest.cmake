include(FetchContent)

FetchContent_Populate(
  googletest
  URL      https://github.com/google/googletest/archive/refs/tags/release-1.8.0.zip
  URL_HASH MD5=adfafc8512ab65fd3cf7955ef0100ff5
  SOURCE_DIR "${CMAKE_BINARY_DIR}/googletest"
)
