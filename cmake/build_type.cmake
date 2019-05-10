# Checks which build type is active.
#
# Variables:
#  - IS_DEBUG_BUILD: TRUE / FALSE
#  - IS_RELEASE_BUILD: TRUE / FALSE
#  - BUILD_TYPE: Debug / Release
string (TOUPPER ${CMAKE_BUILD_TYPE} build_type)
if (build_type MATCHES "DEBUG")
  set (IS_DEBUG_BUILD TRUE)
  set (IS_RELEASE_BUILD FALSE)
  set (BUILD_TYPE "Debug")
elseif (build_type MATCHES "RELEASE")
  set (IS_DEBUG_BUILD FALSE)
  set (IS_RELEASE_BUILD TRUE)
  set (BUILD_TYPE "Release")
else ()
  message (FATAL_ERROR "Invalid build type ${CMAKE_BUILD_TYPE}. Valid types are Debug and Release.")
endif ()

message (STATUS "Build type is ${BUILD_TYPE}")
