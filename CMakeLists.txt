cmake_minimum_required(VERSION 3.1)
project(XCEC VERSION 0.1.0)

#set(CMAKE_BUILD_TYPE Debug)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check platforms
if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif(CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

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
    link_directories("${PROJECT_SOURCE_DIR}/lib/win") 
elseif(LINUX)
    MESSAGE("link_directories: ${PROJECT_SOURCE_DIR}/lib/linux")
    link_directories("${PROJECT_SOURCE_DIR}/lib/linux")
    find_package(Threads REQUIRED)
elseif(MACOS)
    MESSAGE("link_directories: ${CMAKE_SOURCE_DIR}/lib/macos")
    link_directories("${CMAKE_SOURCE_DIR}/lib/macos") 
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
if(WINDOWS)
    if(CMAKE_CL_64)
        target_link_libraries (${PROJECT_NAME} z3)
    else()
        target_link_libraries (${PROJECT_NAME} z3)
    endif()
elseif(LINUX)
    # static library
    target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_SOURCE_DIR}/lib/linux/libz3.a)
    # shared library
    # target_link_libraries (${PROJECT_NAME} -lz3)
elseif(MACOS)
    # static library
    target_link_libraries (${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/macos/libz3.a)
    # shared library
    # target_link_libraries (${PROJECT_NAME} -lz3)
endif()

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
