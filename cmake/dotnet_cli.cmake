# Dotnet CLI

find_program (DOTNET_EXECUTABLE "dotnet")
if (DOTNET_EXECUTABLE)
  execute_process (
    COMMAND "${DOTNET_EXECUTABLE}" --info
    COMMAND grep -e "RID:.*-.*"
    COMMAND sed "s/.*-//"
    OUTPUT_VARIABLE DOTNET_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  message (STATUS "Dotnet CLI architecture is ${DOTNET_ARCHITECTURE}")
else ()
  message (WARNING "Dotnet CLI not found. Targets disabled.")
endif ()
