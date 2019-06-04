# Ignore all warnings
if (WIN32)
  set (CMAKE_CXX_FLAGS    "/EHsc /permissive- /W4 /w14640 /GL")
  use_static_windows_runtime ()
else ()
  set (CMAKE_CXX_STANDARD 14)
  set (CMAKE_CXX_FLAGS    "-std=c++${CMAKE_CXX_STANDARD} -fPIC -fvisibility=hidden")
endif ()

# json-schema-validator
find_package (nlohmann_json REQUIRED)

add_library (json-schema-validator STATIC
  json-schema-validator/src/json-schema-draft7.json.cpp
  json-schema-validator/src/json-uri.cpp
  json-schema-validator/src/json-validator.cpp
)

target_include_directories (json-schema-validator PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/json-schema-validator/src
)

target_link_libraries (json-schema-validator
  nlohmann_json::nlohmann_json
)

cotire (json-schema-validator)
