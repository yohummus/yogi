# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS "/EHsc /permissive- /W4 /w14640 /GL")
  set (CMAKE_STATIC_LINKER_FLAGS "/LTCG")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
  set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
  use_static_windows_runtime ()

  add_definitions(
    -D_WIN32_WINNT=0x0601 # Windows 7
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
  )
else ()
  set (warnings                  "-Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference")
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-shadow -Wno-trigraphs -Wno-unused-private-field -Wno-unused-parameter")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} ${warnings} ${ignored_warnings} -fPIC -DBOOST_ALL_DYN_LINK -fvisibility=hidden")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG -fwhole-program -flto")
endif ()

# 3rd party libraries
add_subdirectory (3rd_party)

# nlohmann-json
find_package (nlohmann_json REQUIRED)

# OpenSSL
set (OPENSSL_USE_STATIC_LIBS ON)
find_package (OpenSSL REQUIRED)

# Boost
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_DEBUG_RUNTIME ON) # TODO?
set (Boost_MULTITHREADED ON)
find_package (Boost 1.66
  COMPONENTS system filesystem thread date_time program_options
  REQUIRED)
add_library (boost INTERFACE IMPORTED)
set_property (TARGET boost
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
find_package (Threads REQUIRED)

# Static library
set (schema_files
  src/schema/branch.schema.json
  src/schema/web_authentication.schema.json
  src/schema/web_groups.schema.json
  src/schema/web_permissions.schema.json
  src/schema/web_routes.schema.json
  src/schema/web_server.schema.json
  src/schema/web_ssl.schema.json
  src/schema/web_users.schema.json
)

set (gen_schemas_file generated_schema_files_content.cc)

add_custom_command(
    OUTPUT  ${gen_schemas_file}
    COMMAND ${CMAKE_COMMAND}
    -DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/${gen_schemas_file}
    -DSCHEMA_FILES="${schema_files}"
    -DFN_NAME="Generated_GetSchemaFiles"
    -P ${CMAKE_SOURCE_DIR}/cmake/generate_files_loader.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${schema_files}
)

add_library (yogi-core-static STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/${gen_schemas_file}
  src/api/constants.cc
  src/api/errors.cc
  src/api/object.cc
  src/licenses/3rd_party_licenses.cc
  src/licenses/yogi_license.cc
  src/network/ip.cc
  src/network/messages.cc
  src/network/msg_transport.cc
  src/network/tcp_listener.cc
  src/network/tcp_transport.cc
  src/network/transport.cc
  src/objects/branch/detail/advertising_receiver.cc
  src/objects/branch/detail/advertising_sender.cc
  src/objects/branch/detail/branch_connection.cc
  src/objects/branch/detail/branch_info.cc
  src/objects/branch/detail/broadcast_manager.cc
  src/objects/branch/detail/connection_manager.cc
  src/objects/branch/branch.cc
  src/objects/config/detail/command_line_parser.cc
  src/objects/config/configuration.cc
  src/objects/log/detail/console_log_sink.cc
  src/objects/log/detail/file_log_sink.cc
  src/objects/log/detail/hook_log_sink.cc
  src/objects/log/detail/log_sink.cc
  src/objects/log/detail/text_based_log_sink.cc
  src/objects/log/logger.cc
  src/objects/web/detail/auth_provider.cc
  src/objects/web/detail/group.cc
  src/objects/web/detail/http_session.cc
  src/objects/web/detail/mime.cc
  src/objects/web/detail/permissions.cc
  src/objects/web/detail/route.cc
  src/objects/web/detail/ssl_context.cc
  src/objects/web/detail/user.cc
  src/objects/web/detail/websocket_session.cc
  src/objects/web/detail/worker_pool.cc
  src/objects/web/web_process.cc
  src/objects/web/web_route.cc
  src/objects/web/web_server.cc
  src/objects/context.cc
  src/objects/signal_set.cc
  src/objects/timer.cc
  src/schema/schema.cc
  src/utils/console.cc
  src/utils/crypto.cc
  src/utils/glob.cc
  src/utils/json_helpers.cc
  src/utils/ringbuffer.cc
  src/utils/system.cc
  src/utils/timestamp.cc
  src/utils/types.cc
)

target_include_directories (yogi-core-static SYSTEM PUBLIC
  ${Boost_INCLUDE_DIRS}
)

cotire (yogi-core-static)

export (TARGETS yogi-core-static
  FILE YogiCoreStaticConfig.cmake
)

# Dynamic library
add_library (yogi-core SHARED
  src/yogi_core/branches.cc
  src/yogi_core/configurations.cc
  src/yogi_core/contexts.cc
  src/yogi_core/helpers.cc
  src/yogi_core/logging.cc
  src/yogi_core/miscellaneous.cc
  src/yogi_core/objects.cc
  src/yogi_core/signals.cc
  src/yogi_core/time.cc
  src/yogi_core/timers.cc
  src/yogi_core/web.cc
)

target_link_libraries (yogi-core PRIVATE
  yogi-core-static
  ${Boost_LIBRARIES}
  OpenSSL::Crypto
  OpenSSL::SSL
  nlohmann_json::nlohmann_json
  json-schema-validator
)

if (WIN32)
  target_link_libraries (yogi-core
    bcrypt.lib
    IPHLPAPI.lib
  )
endif ()

if (WIN32)
  set_target_properties (yogi-core PROPERTIES
    OUTPUT_NAME "yogi-core-${TARGET_ARCHITECTURE}"
  )
endif()

cotire (yogi-core)

target_include_directories (yogi-core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src
)

export (TARGETS yogi-core
  FILE YogiCoreConfig.cmake
)

# Core header only
add_library (yogi-core-header INTERFACE)

target_include_directories (yogi-core-header INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

export (TARGETS yogi-core-header
  FILE YogiCoreHeaderConfig.cmake
)

# Tests
find_package (GMock REQUIRED)
find_package (GTest REQUIRED)
include_directories (SYSTEM ${GMOCK_INCLUDE_DIRS})

add_executable (yogi-core-test
  test/api/constants_test.cc
  test/api/errors_test.cc
  test/api/object_test.cc
  test/licenses/licenses_test.cc
  test/network/messages_test.cc
  test/network/msg_transport_test.cc
  test/network/serialize_test.cc
  test/network/tcp_transport_test.cc
  test/network/tcp_listener_test.cc
  test/network/transport_test.cc
  test/objects/branch/branch_test.cc
  test/objects/branch/broadcast_manager_test.cc
  test/objects/branch/connection_manager_test.cc
  test/objects/configuration/command_line_parser_test.cc
  test/objects/configuration/configuration_test.cc
  test/objects/web/auth_provider_test.cc
  test/objects/web/mime_test.cc
  test/objects/web/permissions_test.cc
  test/objects/web/routes_test.cc
  test/objects/web/ssl_context_test.cc
  test/objects/web/web_server_test.cc
  test/objects/web/worker_pool_test.cc
  test/objects/log/logger_test.cc
  test/objects/context_test.cc
  test/objects/format_test.cc
  test/objects/signal_set_test.cc
  test/objects/timer_test.cc
  test/utils/algorithm_test.cc
  test/utils/crypto_test.cc
  test/utils/glob_test.cc
  test/utils/ringbuffer_test.cc
  test/utils/system_test.cc
  test/common.cc
  test/time_test.cc
  test/version_test.cc
)

get_filename_component (test-data-dir "test/data" REALPATH)
target_compile_definitions (yogi-core-test
  PRIVATE YOGI_TEST_DATA_DIR="${test-data-dir}"
)

target_link_libraries (yogi-core-test
  yogi-core-static
  yogi-core
  ${GMOCK_BOTH_LIBRARIES}
  ${GTEST_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
  nlohmann_json::nlohmann_json
  json-schema-validator
)

cotire (yogi-core-test)

add_test (yogi-core-test yogi-core-test)

# Valgrind
find_program (VALGRIND_EXECUTABLE, "valgrind")

if (VALGRIND_EXECUTABLE)
  add_custom_target(yogi-core-test-valgrind
    COMMAND ${VALGRIND_EXECUTABLE} --leak-check=yes $<TARGET_FILE:yogi-core-test>
    DEPENDS yogi-core-test
  )

  add_custom_target(yogi-core-test-helgrind
    COMMAND ${VALGRIND_EXECUTABLE} --tool=helgrind $<TARGET_FILE:yogi-core-test>
    DEPENDS yogi-core-test
  )
else ()
  message (STATUS "Valgrind not found. Targets disabled.")
endif ()

# Documentation
add_custom_target (yogi-core-doc
  COMMAND doxygen
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/doc
)
