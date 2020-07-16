#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "stp" for configuration "Release"
set_property(TARGET stp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

if(WINDOWS)

elseif(LINUX)
  set_target_properties(stp PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
    IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/linux/libstp.a"
    )

  list(APPEND _IMPORT_CHECK_TARGETS stp )
  list(APPEND _IMPORT_CHECK_FILES_FOR_stp "${_IMPORT_PREFIX}/lib/linux/libstp.a" )
elseif(MACOS)
  set_target_properties(stp PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
    IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/mac/libstp.2.3.dylib"
    IMPORTED_SONAME_RELEASE "libstp.dylib"
    )

  list(APPEND _IMPORT_CHECK_TARGETS stp )
  list(APPEND _IMPORT_CHECK_FILES_FOR_stp "${_IMPORT_PREFIX}/lib/mac/libstp.2.3.dylib" )
endif()



# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
