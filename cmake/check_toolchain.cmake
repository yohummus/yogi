# Check that vcpkg toolchain file and a supported triplet has been supplied.
#
# Variables:
#  - TARGET_ARCHITECTURE: x86, x64, arm or arm64

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message (FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not defined. Please run CMake \
    with -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif ()

if (NOT DEFINED VCPKG_TARGET_TRIPLET)
    message (FATAL_ERROR "VCPKG_TARGET_TRIPLET is not defined. Please run CMAKE \
      with -DVCPKG_TARGET_TRIPLET=...")
  endif()

if (${VCPKG_TARGET_TRIPLET} MATCHES ".*-windows.*")
  if (NOT ${VCPKG_TARGET_TRIPLET} MATCHES ".*-windows-static")
    message (FATAL_ERROR "Unsupported VCPKG_TARGET_TRIPLET value. Please use \
      one of the ...-windows-static triplets.")
  endif ()
endif ()

get_filename_component (triplet_path ${CMAKE_TOOLCHAIN_FILE} DIRECTORY)
get_filename_component (triplet_path ${triplet_path}, DIRECTORY)
get_filename_component (triplet_path ${triplet_path}, DIRECTORY)
set (triplet_path "${triplet_path}/triplets/${VCPKG_TARGET_TRIPLET}.cmake")

if (NOT EXISTS "${triplet_path}")
  message (FATAL_ERROR "Target triplet ${triplet_path}: No such file or directory")
endif ()

set (regex "set *\\( *VCPKG_TARGET_ARCHITECTURE *(.*) *\\)")
file (
  STRINGS "${triplet_path}" temp
  REGEX ${regex}
)

string (REGEX MATCH ${regex} temp ${temp})
set (TARGET_ARCHITECTURE ${CMAKE_MATCH_1})

message (STATUS "Target architecture is ${TARGET_ARCHITECTURE}")
