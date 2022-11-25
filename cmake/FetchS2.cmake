include(FetchContent)

FetchContent_Declare(
  s2
  GIT_REPOSITORY https://github.com/google/s2geometry.git
  GIT_TAG v0.10.0-2022.02.23.00
)

FetchContent_MakeAvailable(s2)
