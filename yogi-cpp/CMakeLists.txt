# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS "/EHsc /permissive- /W4 /w14640")
  use_static_windows_runtime ()
else ()
  set (warnings                  "-Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference")
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-shadow -Wno-trigraphs -Wno-unused-private-field -Wno-unused-parameter")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} ${warnings} ${ignored_warnings}")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
endif ()

# Interface library
add_library (yogi-cpp INTERFACE)

target_link_libraries (yogi-cpp INTERFACE
  yogi-core-header
  ${CMAKE_DL_LIBS}
)

target_include_directories (yogi-cpp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

export (TARGETS yogi-cpp
  FILE YogiCppConfig.cmake
)

# Tests
find_package (GTest REQUIRED)
include_directories (SYSTEM ${GTEST_INCLUDE_DIRS})
add_definitions (-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

find_package (Boost 1.66
  COMPONENTS filesystem system
  REQUIRED)

add_executable (yogi-cpp-test
  test/compile_tests/bindings_info_compile_test.cc
  test/compile_tests/branch_compile_test.cc
  test/compile_tests/buffer_compile_test.cc
  test/compile_tests/configuration_compile_test.cc
  test/compile_tests/constants_compile_test.cc
  test/compile_tests/context_compile_test.cc
  test/compile_tests/duration_compile_test.cc
  test/compile_tests/errors_compile_test.cc
  test/compile_tests/io_compile_test.cc
  test/compile_tests/json_compile_test.cc
  test/compile_tests/json_view_compile_test.cc
  test/compile_tests/licenses_compile_test.cc
  test/compile_tests/logging_compile_test.cc
  test/compile_tests/msgpack_compile_test.cc
  test/compile_tests/msgpack_view_compile_test.cc
  test/compile_tests/object_compile_test.cc
  test/compile_tests/operation_id_compile_test.cc
  test/compile_tests/payload_view_compile_test.cc
  test/compile_tests/signals_compile_test.cc
  test/compile_tests/stream_io_compile_test.cc
  test/compile_tests/string_view_compile_test.cc
  test/compile_tests/time_compile_test.cc
  test/compile_tests/timer_compile_test.cc
  test/compile_tests/timestamp_compile_test.cc
  test/compile_tests/uuid_compile_test.cc
  test/compile_tests/version_compile_test.cc
  test/internal_tests/duration_conversion_test.cc
  test/public_api_tests/bindings_info_test.cc
  test/public_api_tests/branch_test.cc
  test/public_api_tests/configuration_test.cc
  test/public_api_tests/constants_test.cc
  test/public_api_tests/context_test.cc
  test/public_api_tests/duration_test.cc
  test/public_api_tests/errors_test.cc
  test/public_api_tests/io_test.cc
  test/public_api_tests/json_test.cc
  test/public_api_tests/json_view_test.cc
  test/public_api_tests/licenses_test.cc
  test/public_api_tests/logging_test.cc
  test/public_api_tests/msgpack_test.cc
  test/public_api_tests/msgpack_view_test.cc
  test/public_api_tests/object_test.cc
  test/public_api_tests/operation_id_test.cc
  test/public_api_tests/payload_view_test.cc
  test/public_api_tests/signals_test.cc
  test/public_api_tests/stream_io_test.cc
  test/public_api_tests/string_view_test.cc
  test/public_api_tests/time_test.cc
  test/public_api_tests/timer_test.cc
  test/public_api_tests/timestamp_test.cc
  test/public_api_tests/uuid_test.cc
  test/public_api_tests/version_test.cc
  test/public_api_tests/yogi_header_test.cc
  test/common.cc
)

target_link_libraries (yogi-cpp-test
  yogi-cpp
  ${GTEST_BOTH_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
)

cotire (yogi-cpp-test)

add_test (
  NAME yogi-cpp-test
  COMMAND yogi-cpp-test
)

set_property (TEST yogi-cpp-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)

# Documentation
add_custom_target (yogi-cpp-doc
  COMMAND doxygen
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/doc
)
