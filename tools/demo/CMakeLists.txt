project(rhhp_demo)

set(BIN_TARGET rhhp_demo)
add_executable(${BIN_TARGET} main.cpp  urls_large_data.cpp  urls_large_data.hpp)
if (ENABLE_SANITIZERS)
    add_sanitizers(${BIN_TARGET})
endif()
target_link_libraries(${BIN_TARGET} rhhp)


include(RhhpUtil)
rhhp_setup_compiler(${BIN_TARGET})

install(TARGETS ${BIN_TARGET} DESTINATION ${BINARY_INSTALL_DIR})
