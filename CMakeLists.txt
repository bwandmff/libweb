cmake_minimum_required(VERSION 3.10)
project(libweb_server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 查找 libwebsockets 库
find_library(LWS_LIB NAMES websockets)
find_path(LWS_INCLUDE_DIR libwebsockets.h)

if(NOT LWS_LIB OR NOT LWS_INCLUDE_DIR)
    # 如果找不到，则假定在标准位置
    message(WARNING "Could not find libwebsockets via find_library/find_path, using standard paths")
    set(LWS_LIB websockets)
    set(LWS_INCLUDE_DIR "/usr/include")
endif()

# 显示找到的信息
message(STATUS "Found libwebsockets: ${LWS_LIB}")
message(STATUS "Found libwebsockets headers: ${LWS_INCLUDE_DIR}")

# Include directories
include_directories(${LWS_INCLUDE_DIR})

# Create the executable
add_executable(libweb_server
    src/server.c
)

# Link libraries
target_link_libraries(libweb_server
    ${LWS_LIB}
)

# Install targets
install(TARGETS libweb_server DESTINATION bin)