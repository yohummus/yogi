find_program (NODEJS_EXECUTABLE "node")
find_program (NPM_EXECUTABLE "npm")

if (NODEJS_EXECUTABLE)
  message (STATUS "NodeJS found")
else ()
  message (STATUS "NodeJS not found. Targets disabled.")
endif ()

if (NPM_EXECUTABLE)
  message (STATUS "NPM found")
else ()
  message (STATUS "NPM not found. Targets disabled.")
endif ()
