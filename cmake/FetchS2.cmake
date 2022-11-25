include(FetchContent)

FetchContent_Declare(
  s2
  GIT_REPOSITORY https://github.com/google/s2geometry.git
  GIT_TAG ad1489e898f369ca09e2099353ccd55bd0fd7a26
)

FetchContent_MakeAvailable(s2)
