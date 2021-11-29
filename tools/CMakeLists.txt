project(hhp_tools)

option(BUILD_TOOLS_DEMO "Build demo tool for parallel fetch" ON)
option(BUILD_TOOLS_HTTPCAT "Build HTTP cat tool (wget like)" ON)

if (BUILD_TOOLS_DEMO)
    add_subdirectory(demo)
endif()
if (BUILD_TOOLS_HTTPCAT)
    add_subdirectory(httpcat)
endif()
