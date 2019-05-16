cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (check_toolchain)
include (yogi_version)

project (yogi VERSION ${YOGI_VERSION_NO_SUFFIX} LANGUAGES CXX)

include (build_type)
include (cotire)
include (GNUInstallDirs)
include (static_windows_runtime)
include (python_interpreter)
include (dotnet_cli)
include (nodejs)

enable_testing ()

# Sub-projects
add_subdirectory (yogi-core)
add_subdirectory (yogi-cpp)
add_subdirectory (yogi-hub)

if (PYTHON_EXECUTABLE)
  add_subdirectory (yogi-python)
endif ()

if (DOTNET_EXECUTABLE)
  add_subdirectory (yogi-dotnet)
endif ()

if (NODEJS_EXECUTABLE AND NPM_EXECUTABLE)
  add_subdirectory (yogi-javascript)
endif ()

# Documentation
add_custom_target (doc)
add_dependencies (doc yogi-core-doc yogi-cpp-doc)
