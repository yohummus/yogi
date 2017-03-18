cmake_minimum_required(VERSION 2.8)
project(YogiPython NONE)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

enable_testing()

#===== install package =====
set(yogi_hub_base /usr/share/yogi-hub/www)
file(GLOB files build/*.min.js build/*.min.js.map)

install(
    CODE "execute_process(COMMAND python3 setup.py install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})"
)

#===== tests =====
file(GLOB_RECURSE files ${CMAKE_SOURCE_DIR}/tests/test_*.py)
foreach(file ${files})
    get_filename_component(name ${file} NAME_WE)
    add_test(NAME ${name}_py3 COMMAND python3 ${file})
endforeach()
