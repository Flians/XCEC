#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cryptominisat5" for configuration "RelWithDebInfo"
set_property(TARGET cryptominisat5 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)

if(WINDOWS)
elseif(LINUX)
  set_target_properties(cryptominisat5 PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
    IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/linux/libcryptominisat5.a"
    )

  list(APPEND _IMPORT_CHECK_TARGETS cryptominisat5 )
  list(APPEND _IMPORT_CHECK_FILES_FOR_cryptominisat5 "${_IMPORT_PREFIX}/lib/linux/libcryptominisat5.a" )
elseif(MACOS)
  set_target_properties(cryptominisat5 PROPERTIES
    IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/mac/libcryptominisat5.5.7.dylib"
    IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libcryptominisat5.5.7.dylib"
    )

  list(APPEND _IMPORT_CHECK_TARGETS cryptominisat5 )
  list(APPEND _IMPORT_CHECK_FILES_FOR_cryptominisat5 "${_IMPORT_PREFIX}/lib/mac/libcryptominisat5.5.7.dylib" )
endif()
# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
