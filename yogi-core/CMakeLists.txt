cmake_minimum_required (VERSION 2.8)
project(CHIRP)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

find_package(Threads)
set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost 1.54 COMPONENTS system log log_setup filesystem thread REQUIRED)

set(ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-unused-private-field -Wno-inconsistent-missing-override -Wno-unknown-warning-option")
set(export_map                "-Wl,--version-script='${CMAKE_SOURCE_DIR}/src/export.map'")
set(CMAKE_CXX_FLAGS           "-std=c++14 -Wall -Werror ${ignored_warnings} -fvisibility=hidden -fPIC -DBOOST_ALL_DYN_LINK")
set(CMAKE_CXX_FLAGS_DEBUG     "-g")
set(CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
set(CMAKE_SHARED_LINKER_FLAGS "${export_map}")

file(GLOB_RECURSE dll_files           src/chirp.cpp)
file(GLOB_RECURSE include_files       src/chirp.h)
file(GLOB_RECURSE lib_files           src/config.h src/**/*.hpp src/**/*.cpp)
file(GLOB_RECURSE common_tests_files  tests/helpers/*.hpp tests/helpers/*.cpp tests/mocks/*.hpp)
file(GLOB_RECURSE unit_tests_files    tests/unit_tests/*.cpp)
file(GLOB_RECURSE library_tests_files tests/library_tests/*.cpp)
file(GLOB_RECURSE stress_tests_files  tests/stress_tests/*.cpp)

add_subdirectory(/usr/src/googletest googletest)
enable_testing()

#===== libchirp_lib.a =====
add_library(chirp_lib STATIC ${include_files} ${lib_files})

#===== libchirp.so =====
add_library(chirp SHARED ${dll_files})
target_link_libraries(chirp ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} chirp_lib)

install(TARGETS chirp DESTINATION lib)
install(FILES src/chirp.h DESTINATION include)

#===== unit_tests =====
add_executable(unit_tests ${unit_tests_files} ${common_tests_files} ${chirp})
target_link_libraries(unit_tests chirp_lib chirp gmock gmock_main ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
add_test(NAME unit_tests COMMAND unit_tests)

#===== library_tests =====
add_executable(library_tests ${library_tests_files} ${common_tests_files} ${chirp})
target_link_libraries(library_tests chirp gmock gmock_main)
add_test(NAME library_tests COMMAND library_tests)

#===== stress_tests =====
add_executable(stress_tests ${stress_tests_files} ${common_tests_files} ${chirp})
target_link_libraries(stress_tests chirp gmock gmock_main)
add_test(NAME stress_tests COMMAND stress_tests)
