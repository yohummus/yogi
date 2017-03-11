cmake_minimum_required(VERSION 2.8)
project(YogiCpp)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

find_package(Threads)
set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost 1.54 COMPONENTS system filesystem program_options REQUIRED)
find_package(Protobuf REQUIRED)

set(ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value")
set(CMAKE_CXX_FLAGS           "-std=c++14 -Wall -Werror ${ignored_warnings} -fvisibility=hidden -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG     "-g")
set(CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")

add_subdirectory(/usr/src/googletest googletest EXCLUDE_FROM_ALL)
enable_testing()

#===== libyogi_cpp.a =====
file(GLOB_RECURSE files yogi_cpp/*.h yogi_cpp/*.hpp yogi_cpp/*.cpp yogi_cpp/*.cc)
add_library(yogi_cpp STATIC ${files})

install(TARGETS yogi_cpp DESTINATION lib)
install(FILES yogi_cpp.hpp DESTINATION include)
file(GLOB files yogi_cpp/*.hpp)
install(FILES ${files} DESTINATION include/yogi_cpp)
file(GLOB files yogi_cpp/internal/*.hpp)
install(FILES ${files} DESTINATION include/yogi_cpp/internal)

#===== tests =====
file(GLOB_RECURSE files tests/*.h tests/*.hpp tests/*.cpp tests/*.cc)
add_executable(tests ${files} ${yogi_cpp})
target_link_libraries(tests yogi_cpp yogi_core gtest gtest_main ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} pthread)
add_test(NAME tests COMMAND tests)
