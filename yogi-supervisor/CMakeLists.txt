cmake_minimum_required (VERSION 2.8)
project (YogiSupervisor)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

find_package (Threads)
set (Boost_USE_STATIC_LIBS        OFF)
set (Boost_USE_MULTITHREADED      ON)
set (Boost_USE_STATIC_RUNTIME     OFF)
find_package (Boost 1.54 COMPONENTS program_options log log_setup system thread filesystem REQUIRED)
find_package (Protobuf 3.0.0 REQUIRED)

set (ignored_warnings        "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value")
set (CMAKE_CXX_FLAGS         "-std=c++14 -Wall -Werror ${ignored_warnings} -DBOOST_ALL_DYN_LINK")
set (CMAKE_CXX_FLAGS_DEBUG   "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

include_directories (${Boost_INCLUDE_DIRS})
include_directories (${PROTOBUF_INCLUDE_DIRS})
file (GLOB_RECURSE src_files src/*.cc src/*.cpp src/*.hh src/*.hpp)

#===== build executable =====
add_executable (yogi-supervisor ${src_files})
target_link_libraries (yogi-supervisor yogi yogi_core ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

#===== tests =====
enable_testing()

file(GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/tests/test_*.py)
foreach(file ${files})
    file(RELATIVE_PATH rel_file ${PROJECT_SOURCE_DIR} ${file})
    get_filename_component(name ${file} NAME_WE)

    add_test(NAME ${name} COMMAND python3 ${file})
endforeach()

#===== install =====
install (
    TARGETS yogi-supervisor
    DESTINATION /usr/bin
)

install (
    FILES yogi-supervisor.json
    DESTINATION /etc
)

install (
    FILES yogi-supervisor.sh
    DESTINATION /etc/init.d
    RENAME yogi-supervisor
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)
