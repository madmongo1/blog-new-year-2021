cmake_minimum_required(VERSION 3.17)

project(rhhpl_project VERSION 1.0.0 DESCRIPTION "Richard Hodges's HTTP pool library")

option(MULTI_THREADED OFF)
option(ENABLE_SANITIZERS OFF)
if (ENABLE_SANITIZERS)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})
    option(SANITIZE_ADDRESS OFF)
    option(SANITIZE_MEMORY OFF)
    option(SANITIZE_THREAD OFF)
    option(SANITIZE_UNDEFINED OFF)
    find_package(Sanitizers)
else()
    function(add_sanitizers)
    endfunction()
endif()

option(BUILD_TOOLS "Build example tools" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(GNUInstallDirs)

add_subdirectory(src)
if (BUILD_TOOLS)
    add_subdirectory(tools)
endif()
