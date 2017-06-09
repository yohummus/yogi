cmake_minimum_required (VERSION 2.8)
project (YogiPython NONE)

#===== convert to Python 2 (pasteurize) =====
set (output_files "")
file (GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/setup.py ${PROJECT_SOURCE_DIR}/yogi/*.py ${PROJECT_SOURCE_DIR}/tests/*.py ${PROJECT_SOURCE_DIR}/tests/*.json)
foreach (file ${files})
    # workaround; seems to be a but in CMake
    if (${file} MATCHES .*/build/setup.py)
        continue ()
    endif ()

    file (RELATIVE_PATH output ${PROJECT_SOURCE_DIR} ${file})
    get_filename_component (dir ${output} DIRECTORY)
    list (APPEND output_files ${output})

    if (${output} MATCHES .*.json)
        add_custom_command (
            COMMENT "copying ${output}"
            DEPENDS ${file}
            OUTPUT ${output}
            COMMAND cp ${file} ${output}
        )
    else ()
        add_custom_command (
            COMMENT "pasteurizing ${output}"
            DEPENDS ${file}
            OUTPUT ${output}
            COMMAND mkdir -p ./${dir}
            COMMAND cp ${file} ${output}
            COMMAND pasteurize -w -n --no-diff ${output} >> pasteurize.log 2>&1
        )
    endif ()
endforeach ()

add_custom_target (pasteurization
    ALL
    DEPENDS ${output_files}
)

#===== tests =====
enable_testing ()

file (GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/tests/test_*.py)
foreach (file ${files})
    file (RELATIVE_PATH rel_file ${PROJECT_SOURCE_DIR} ${file})
    get_filename_component (name ${file} NAME_WE)

    add_test (NAME ${name}_py3 COMMAND python3 ${file})
    add_test (NAME ${name}_py2 COMMAND python2 ${rel_file})
endforeach ()

#===== install package =====
install (
    CODE "execute_process(COMMAND python3 setup.py install WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})"
)

install (
    CODE "execute_process(COMMAND python2 setup.py install WORKING_DIRECTORY ${PROJECT_BINARY_DIR})"
)
