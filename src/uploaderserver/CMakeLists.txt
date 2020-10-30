# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.8)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-D_UNICODE -DUNICODE)

# 包含文件
include_directories(.)

# 遍历匹配目录的所有文件以及子目录下面的文件
file(GLOB_RECURSE uploaderserver_src "*.cpp")
file(GLOB_RECURSE uploaderserver_inc "*.h")

# vs分组
source_group("src" FILES ${uploaderserver_src})
source_group("inc" FILES ${uploaderserver_inc})

# 生成目标
add_executable(uploaderserver ${uploaderserver_src} ${uploaderserver_inc})

