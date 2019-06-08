file (WRITE ${OUT_FILE}
"#include <map>
#include <string>

std::map<std::string, std::string> ${FN_NAME}() {
  return {
"
)

separate_arguments (SCHEMA_FILES)
foreach (schema_file ${SCHEMA_FILES})
  get_filename_component (FILENAME ${schema_file} NAME)
  file (READ ${schema_file} CONTENT)
  file (APPEND ${OUT_FILE} "    {\"${FILENAME}\", R\"SCHEMA(\n${CONTENT}\n    )SCHEMA\"},\n")
endforeach ()

file (APPEND ${OUT_FILE}
"  };
}
"
)