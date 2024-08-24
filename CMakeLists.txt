cmake_minimum_required(VERSION 2.5) 

project(muduo_rpc)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


# 设置.h文件搜索路径 
include_directories(${PROJECT_SOURCE_DIR}/net/include)
include_directories(${PROJECT_SOURCE_DIR}/rpc/include)
include_directories(${PROJECT_SOURCE_DIR}/examples/rpc/userproto)
include_directories(${PROJECT_SOURCE_DIR}/examples/rpc/friendproto)
# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)


add_subdirectory(net)
add_subdirectory(examples)
add_subdirectory(rpc)