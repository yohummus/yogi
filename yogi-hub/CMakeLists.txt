# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS "/EHsc /permissive- /W4 /w14640")
  use_static_windows_runtime ()
else ()
  set (warnings                  "-Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference")
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-unknown-pragmas -Wno-shadow -Wno-trigraphs -Wno-unused-private-field -Wno-unused-parameter")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} ${warnings} ${ignored_warnings}")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
endif ()

# Static library
add_library (yogi-hub-static STATIC
  src/random_stuff.cc
)

cotire (yogi-hub-static)

# Executable
add_executable (yogi-hub
  src/main.cc
)

target_link_libraries (yogi-hub
  yogi-cpp
)

cotire (yogi-hub)

# Tests
find_package (GTest REQUIRED)
include_directories (SYSTEM ${GTEST_INCLUDE_DIRS})
add_definitions (-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

add_executable (yogi-hub-test
  test/yogi_hub_test.cc
)

target_link_libraries (yogi-hub-test
  yogi-cpp
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-hub-test)

add_test (
  NAME yogi-hub-test
  COMMAND yogi-hub-test
)

set_property (TEST yogi-hub-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
