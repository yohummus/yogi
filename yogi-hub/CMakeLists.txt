cmake_minimum_required(VERSION 2.8)
project(YogiHub)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package (Qt5Core REQUIRED)
find_package (Qt5Network REQUIRED)
find_package (Qt5WebSockets REQUIRED)
find_package (Protobuf REQUIRED)
find_package (Boost 1.54 COMPONENTS system filesystem program_options REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5WebSockets_INCLUDE_DIRS})

set(ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value")
set(CMAKE_CXX_FLAGS           "-std=c++14 -Wall -Werror ${ignored_warnings} -fvisibility=hidden -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG     "-g")
set(CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
set(CMAKE_CXX_STANDARD        14)

file(GLOB_RECURSE src_files src/*.hpp src/*.cpp src/*.h src/*.cc)

#===== yogi-hub =====
add_executable(yogi-hub ${src_files})
target_link_libraries(yogi-hub yogi_core yogi_cpp ${PROTOBUF_LIBRARIES} ${Boost_LIBRARIES} Qt5::Core Qt5::Network Qt5::WebSockets)

#===== install =====
install(
    TARGETS yogi-hub
    DESTINATION /usr/bin
)

install(
    FILES yogi-hub.json
    DESTINATION /etc
)

install(
    FILES yogi-hub.sh
    DESTINATION /etc/init.d
    RENAME yogi-hub
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)
