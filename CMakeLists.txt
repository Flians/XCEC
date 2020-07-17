cmake_minimum_required(VERSION 3.1)
project(XCEC VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_BUILD_TYPE Debug)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
else()
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 
endif()
message("Build type: " ${CMAKE_BUILD_TYPE})

# Check platforms
if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif(CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
    message("The enviriment is in linux.")
endif()

add_library(croaring STATIC ${PROJECT_SOURCE_DIR}/include/CRoaring/roaring.c)

option(SHAREDLIBRARY "Use the shared library for z3" OFF)
if (SHAREDLIBRARY)
    set(USESHARED 1)
    message("Use the shared library.")
endif()

# find cryptominisat5
set(cryptominisat5_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/cryptominisat5")
find_package(cryptominisat5 REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/cryptominisat5")
message("Using cryptominisat5 rooted at ${cryptominisat5_DIR}")

# find STP
set(STP_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/STP" CACHE PATH "Try to use particular STP install (set this to folder where STPConfig.cmake is installed)")
find_package(STP REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/STP")
message("Using STP rooted at ${STP_DIR}")

# find z3
set(Z3_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/z3")
find_package(Z3 REQUIRED PATHS "${CMAKE_CURRENT_LIST_DIR}/lib/cmake/z3")
message("Using Z3 rooted at ${Z3_DIR}")

# Set RPATH
if(WINDOWS)
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    MESSAGE( STATUS "CMAKE_INSTALL_RPATH: " "${CMAKE_INSTALL_PREFIX}/lib" )
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Add search path for lib files
if(WINDOWS)
    MESSAGE("link_directories: ${PROJECT_SOURCE_DIR}/lib/win")
    link_directories(${PROJECT_SOURCE_DIR}/lib/win)
elseif(LINUX)
    MESSAGE("link_directories: ${PROJECT_SOURCE_DIR}/lib/linux")
    link_directories(${PROJECT_SOURCE_DIR}/lib//linux)
elseif(MACOS)
    MESSAGE("link_directories: ${CMAKE_SOURCE_DIR}/lib/mac")
    link_directories(${CMAKE_SOURCE_DIR}/lib/mac)
endif()

# the function is used to add all header files
function(include_sub_directories_recursively root_dir)
    if (IS_DIRECTORY ${root_dir})
        message("include dir: " ${root_dir})
        include_directories(${root_dir})
    endif()

    file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*)
    foreach(sub ${ALL_SUB})
        if (IS_DIRECTORY ${root_dir}/${sub})
            include_sub_directories_recursively(${root_dir}/${sub})
        endif()
    endforeach()
endfunction()

# add all header files
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
message("Source file: " ${SOURCE_FILES})

set(XCEC_ALL_SRCS main.cpp ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${XCEC_ALL_SRCS})
target_link_libraries (${PROJECT_NAME} ${Z3_LIBRARIES} stp croaring)

if(WINDOWS)
    # Copy DLL files to output directory
    if(CMAKE_CL_64)
        add_custom_command(TARGET BarcodeReader POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/win/z3.dll"              
        $<TARGET_FILE_DIR:z3>)
    else()
        add_custom_command(TARGET BarcodeReader POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/win/z3.dll"              
        $<TARGET_FILE_DIR:z3>)
    endif()
endif()

# Set installation directory
if(WINDOWS)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_NAME})
    if(CMAKE_CL_64)
    install (FILES "${PROJECT_SOURCE_DIR}/lib/win/z3.dll" DESTINATION bin)
    else()
    install (FILES "${PROJECT_SOURCE_DIR}/lib/win/z3.dll" DESTINATION bin)
    endif()
elseif(LINUX)
    # ldd ./build/XCEC
    install (FILES "${PROJECT_SOURCE_DIR}/lib/linux/libz3.so" DESTINATION lib)
elseif(MACOS)
    # otool -L ./build/XCEC 
    # install_name_tool -change libz3.dylib ./lib/libz3.dylib ./build/XCEC 
    install (FILES "${PROJECT_SOURCE_DIR}/lib/macos/libz3.dylib" DESTINATION lib)
endif()

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (DIRECTORY "${PROJECT_SOURCE_DIR}/include" DESTINATION include)
