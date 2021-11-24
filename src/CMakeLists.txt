project(rhhp_lib VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES CXX)

set(LIB_TARGET rhhp)

find_package(Boost COMPONENTS system REQUIRED)
if (Boost_VERSION VERSION_LESS 1.75)
    message(FATAL_ERROR "Boost 1.75 or higher required")
endif ()
find_package(OpenSSL COMPONENTS)
find_package(Threads)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_library(${LIB_TARGET} STATIC ${HEADERS} ${SOURCES})
if (ENABLE_SANITIZERS)
    add_sanitizers(${LIB_TARGET})
endif()
if(MULTI_THREADED)
    target_compile_definitions(${LIB_TARGET} PUBLIC MULTI_THREADED)
endif()
target_link_libraries(${LIB_TARGET} PUBLIC
        Boost::system
        OpenSSL::SSL OpenSSL::Crypto
        Threads::Threads)
target_include_directories(${LIB_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(RhhpUtil)
rhhp_setup_compiler(${LIB_TARGET})

install(TARGETS ${LIB_TARGET} DESTINATION ${LIBRARY_INSTALL_DIR})

install(FILES
 ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_TARGET}
)
