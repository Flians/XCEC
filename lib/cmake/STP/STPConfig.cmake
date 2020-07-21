# Config file for the installed STP Package
# It defines the following variables
#  STP_INCLUDE_DIRS - include directories for STP
#  STP_STATIC_LIBRARY - static library target (empty if not built)
#  STP_SHARED_LIBRARY - dynamic library target (empty if not built)
#  STP_EXECUTABLE     - the stp executable
#  STP_VERSION_*      - the stp MAJOR/MINOR/PATCH versions
#  STP_VERSION        - the stp complete version string

# Compute paths
get_filename_component(STP_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(STP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")


if ("ON" STREQUAL "ON")
    include(CMakeFindDependencyMacro)
    find_dependency(cryptominisat5)
endif()

# Our library dependencies (contains definitions for IMPORTED targets)
include("${STP_CMAKE_DIR}/STPTargets.cmake")

# These are IMPORTED targets created by STPTargets.cmake
set(STP_EXECUTABLE stp)

set(STP_STATIC_LIBRARY stp)
set(STP_SHARED_LIBRARY stp)

# Note version variables are implicitly set by STPConfigVersion.cmake
