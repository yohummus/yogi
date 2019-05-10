# Download node modules
set (npm_ok_file
  ${CMAKE_CURRENT_BINARY_DIR}/node_modules_installed_successfully)

add_custom_command (
  OUTPUT ${npm_ok_file}
  COMMAND ${NPM_EXECUTABLE} install && echo > ${npm_ok_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Build JS bundle
set (bundle_files
  yogi.js
  yogi.js.map
)

file (GLOB_RECURSE src_files
  src/*.js
  src/**/*.js
)

add_custom_command (
  OUTPUT ${bundle_files}
  COMMAND "${NPM_EXECUTABLE}" run build -- --output-path=${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${src_files} ${npm_ok_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target (yogi-javascript
  ALL
  DEPENDS ${bundle_files}
)

# Tests
if (WIN32)
  set (run_npm_test_script ${CMAKE_CURRENT_BINARY_DIR}/run-npm-test.bat)

  file (
    WRITE ${run_npm_test_script}
    "\"${NPM_EXECUTABLE}\" test"
  )

  add_test (
    NAME yogi-javascript-test
    COMMAND ${run_npm_test_script}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
else ()
  add_test (
    NAME yogi-javascript-test
    COMMAND "${NPM_EXECUTABLE}" test
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  )
endif ()
