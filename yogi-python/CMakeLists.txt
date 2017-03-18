cmake_minimum_required(VERSION 2.8)
project(YogiPython NONE)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

enable_testing()

#===== convert to Python 2 (pasteurize) =====
set(output_files "")
file(GLOB_RECURSE files ${CMAKE_SOURCE_DIR}/setup.py ${CMAKE_SOURCE_DIR}/yogi/*.py ${CMAKE_SOURCE_DIR}/tests/*.py ${CMAKE_SOURCE_DIR}/tests/*.json)
foreach(file ${files})
    file(RELATIVE_PATH output ${CMAKE_SOURCE_DIR} ${file})
    get_filename_component(dir ${output} DIRECTORY)
    list(APPEND output_files ${output})

    if (${output} MATCHES .*.json)
        add_custom_command(
            COMMENT "copying ${output}"
            DEPENDS ${file}
            OUTPUT ${output}
            COMMAND cp ${file} ${output}
        )
    else (${output} MATCHES .*.json)
        add_custom_command(
            COMMENT "pasteurizing ${output}"
            DEPENDS ${file}
            OUTPUT ${output}
            COMMAND mkdir -p ./${dir}
            COMMAND cp ${file} ${output}
            COMMAND pasteurize -w -n --no-diff ${output} >> pasteurize.log 2>&1
        )
    endif (${output} MATCHES .*.json)
endforeach()

add_custom_target(pasteurization
    ALL
    DEPENDS ${output_files}
)

#===== tests =====
file(GLOB_RECURSE files ${CMAKE_SOURCE_DIR}/tests/test_*.py)
foreach(file ${files})
    file(RELATIVE_PATH rel_file ${CMAKE_SOURCE_DIR} ${file})
    get_filename_component(name ${file} NAME_WE)

    add_test(NAME ${name}_py3 COMMAND python3 ${file})
    add_test(NAME ${name}_py2 COMMAND python2 ${rel_file})
endforeach()

#===== install package =====
install(
    CODE "execute_process(COMMAND python3 setup.py install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})"
)

install(
    CODE "execute_process(COMMAND python2 setup.py install WORKING_DIRECTORY ${CMAKE_BINARY_DIR})"
)
