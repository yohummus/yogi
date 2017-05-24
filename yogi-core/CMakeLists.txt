cmake_minimum_required (VERSION 2.8)
project(yogi-core)

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
set(export_map                "-Wl,--version-script='${PROJECT_SOURCE_DIR}/src/export.map'")
set(CMAKE_CXX_FLAGS           "-std=c++14 -Wall -Werror ${ignored_warnings} -fvisibility=hidden -fPIC -DBOOST_ALL_DYN_LINK")
set(CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
set(CMAKE_CXX_STANDARD        14)
set(CMAKE_SHARED_LINKER_FLAGS "${export_map}")

add_subdirectory(/usr/src/googletest googletest EXCLUDE_FROM_ALL)
enable_testing()

#===== libcommon.a =====
file(GLOB_RECURSE files src/*.h src/**/*.hpp src/**/*.cpp)
add_library(common STATIC ${files})

#===== libyogi_core.so =====
file(GLOB_RECURSE files src/yogi_core.cpp)
add_library(yogi_core SHARED ${files})
target_link_libraries(yogi_core ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} common)

install(TARGETS yogi_core DESTINATION /usr/lib)
install(FILES src/yogi_core.h DESTINATION /usr/include)

#===== libtestbase.a =====
file(GLOB_RECURSE files tests/helpers/*.hpp tests/helpers/*.cpp tests/mocks/*.hpp)
add_library(testbase STATIC ${files})

#===== unit_tests =====
file(GLOB_RECURSE files tests/unit_tests/*.cpp)
add_executable(unit_tests ${files} ${testbase} ${yogi_core})
target_link_libraries(unit_tests common gmock gmock_main ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} yogi_core)
add_test(NAME unit_tests COMMAND unit_tests)

#===== library_tests =====
file(GLOB_RECURSE files tests/library_tests/*.cpp)
add_executable(library_tests ${files} ${testbase} ${yogi_core})
target_link_libraries(library_tests yogi_core gmock gmock_main)
add_test(NAME library_tests COMMAND library_tests)

#===== stress_tests =====
file(GLOB_RECURSE files tests/stress_tests/*.cpp)
add_executable(stress_tests ${files} ${testbase} ${yogi_core})
target_link_libraries(stress_tests yogi_core gmock gmock_main testbase)
add_test(NAME stress_tests COMMAND stress_tests)
