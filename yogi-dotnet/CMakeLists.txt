# Build library and test assemblies
add_custom_target (yogi-dotnet
  ALL
  COMMAND dotnet build --configuration ${BUILD_TYPE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Tests
add_test (
  NAME yogi-dotnet-test
  COMMAND "${DOTNET_EXECUTABLE}" test --no-restore --configuration ${BUILD_TYPE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test
)

set_property (TEST yogi-dotnet-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)

if (NOT DOTNET_ARCHITECTURE STREQUAL TARGET_ARCHITECTURE)
  message (STATUS "Dotnet architecture (${DOTNET_ARCHITECTURE}) does not match Yogi Core architecture (${TARGET_ARCHITECTURE}). Dotnet tests disabled.")
  set_property (TEST yogi-dotnet-test PROPERTY
    DISABLED TRUE
  )
endif ()
