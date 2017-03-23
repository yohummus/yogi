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
set(CMAKE_CXX_STANDARD        14)

add_subdirectory(/usr/src/googletest googletest EXCLUDE_FROM_ALL)

#===== libyogi_cpp.a =====
file(GLOB_RECURSE files yogi/*.h yogi/*.hpp yogi/*.cpp yogi/*.cc)
add_library(yogi STATIC ${files})

install(TARGETS yogi DESTINATION /usr/lib)
install(FILES yogi.hpp DESTINATION /usr/include)
file(GLOB files yogi/*.hpp)
install(FILES ${files} DESTINATION /usr/include/yogi)
file(GLOB files yogi/internal/*.hpp)
install(FILES ${files} DESTINATION /usr/include/yogi/internal)

#===== tests =====
enable_testing()

file(GLOB_RECURSE files tests/*.h tests/*.hpp tests/*.cpp tests/*.cc)
add_executable(tests ${files} ${yogi_cpp})
target_link_libraries(tests yogi yogi_core gtest gtest_main ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} pthread)
add_test(NAME tests COMMAND tests)
