# Tests
add_test (
  NAME yogi-python-test
  COMMAND "${PYTHON_EXECUTABLE}" -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_property (TEST yogi-python-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)

if (NOT PYTHON_ARCHITECTURE STREQUAL TARGET_ARCHITECTURE)
  message (STATUS "Python architecture (${PYTHON_ARCHITECTURE}) does not match Yogi Core architecture (${TARGET_ARCHITECTURE}). Python tests disabled.")
  set_property (TEST yogi-python-test PROPERTY
    DISABLED TRUE
  )
endif ()

# Pycodestyle (PEP8)
set (pycodestyle_ok_file
  ${CMAKE_CURRENT_BINARY_DIR}/pycodestyle_run_successfully)

file (GLOB_RECURSE src_files
  yogi/*.py
  yogi/**/*.py
  test/*.py
  test/**/*.py
)

add_custom_command (
  OUTPUT ${pycodestyle_ok_file}
  COMMAND ${PYCODESTYLE_EXECUTABLE} ${src_files}
  COMMAND echo > ${pycodestyle_ok_file}
  DEPENDS ${src_files}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

if (PYCODESTYLE_EXECUTABLE)
  add_custom_target (yogi-python-code-style
    ALL
    DEPENDS ${pycodestyle_ok_file}
  )
endif()
