# Read version number from yogi_core.h
#
# Variables:
#  - YOGI_VERSION: Version of the Yogi Core library
#  - YOGI_VERSION_NO_SUFFIX: Version of the Yogi Core library with the version suffix

file (
  STRINGS "${CMAKE_SOURCE_DIR}/yogi-core/include/yogi_core.h" temp
  REGEX "#define YOGI_HDR_VERSION +\".*$"
)

string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+[^ \\s\"]*" YOGI_VERSION ${temp})
string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" YOGI_VERSION_NO_SUFFIX ${YOGI_VERSION})

message (STATUS "Yogi version is ${YOGI_VERSION}")
