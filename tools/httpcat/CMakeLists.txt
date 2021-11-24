project(rhhp_httpcat)

find_package(Boost COMPONENTS program_options REQUIRED)

set(BIN_TARGET httpcat)
add_executable(${BIN_TARGET} main.cpp)
if (ENABLE_SANITIZERS)
    add_sanitizers(${BIN_TARGET})
endif()
target_link_libraries(${BIN_TARGET} rhhp Boost::program_options)

include(RhhpUtil)
rhhp_setup_compiler(${BIN_TARGET})

install(TARGETS ${BIN_TARGET} DESTINATION ${BINARY_INSTALL_DIR})
