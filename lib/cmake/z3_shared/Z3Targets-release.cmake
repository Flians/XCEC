#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "z3::libz3" for configuration "Release"
set_property(TARGET z3::libz3 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

if(WINDOWS)

elseif(LINUX)
  set_target_properties(z3::libz3 PROPERTIES
    IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/linux/libz3.so.4.8.9.0"
    IMPORTED_SONAME_RELEASE "libz3.so.4.8"
    )

  list(APPEND _IMPORT_CHECK_TARGETS z3::libz3 )
  list(APPEND _IMPORT_CHECK_FILES_FOR_z3::libz3 "${_IMPORT_PREFIX}/lib/linux/libz3.so.4.8.9.0" )
elseif(MACOS)
  set_target_properties(z3::libz3 PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
    IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/mac/libz3.dylib"
    )

  list(APPEND _IMPORT_CHECK_TARGETS z3::libz3 )
  list(APPEND _IMPORT_CHECK_FILES_FOR_z3::libz3 "${_IMPORT_PREFIX}/lib/mac/libz3.dylib" )
endif()

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
