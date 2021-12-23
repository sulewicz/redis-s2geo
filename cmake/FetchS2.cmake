include(FetchContent)

FetchContent_Declare(
  s2
  GIT_REPOSITORY https://github.com/google/s2geometry.git
  GIT_TAG        bdaaf97c60b3e29c0eb74dbdc66a7a19f1c937f6
)

FetchContent_MakeAvailable(s2)
