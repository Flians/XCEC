# Try to find the z3 librairies
# Z3_FOUND - system has z3 lib
# Z3_INCLUDE_DIR - the z3 include directory
# Z3_LIBRARY - Libraries needed to use z3

if (Z3_INCLUDE_DIR AND Z3_LIBRARY)
		# Already in cache, be silent
		set(Z3_FIND_QUIETLY TRUE)
endif (Z3_INCLUDE_DIR AND Z3_LIBRARY)

set(Z3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/z3/src)

find_library(Z3_LIBRARY NAMES Z3
        PATHS ${PROJECT_SOURCE_DIR}/lib
        PATH_SUFFIXES "" "lib" "lib64" NO_DEFAULT_PATH
        )
MESSAGE(STATUS "z3 libs: " ${Z3_LIBRARY} )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Z3 DEFAULT_MSG Z3_INCLUDE_DIR Z3_LIBRARY)

mark_as_advanced(Z3_INCLUDE_DIR Z3_LIBRARY)
