include(FetchContent)

FetchContent_Declare(
  s2
  GIT_REPOSITORY https://github.com/google/s2geometry.git
  GIT_TAG v0.10.0
)

FetchContent_MakeAvailable(s2)
