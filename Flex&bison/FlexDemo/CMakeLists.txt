cmake_minimum_required(VERSION 3.0)
project(FlexDemoProject)

# 设置C编译器
set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
# 设置C++编译器
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")

# 添加定义，如果你不需要yywrap可以定义此宏
add_definitions(-DYY_NO_UNISTD_H)

# 设置构建类型为Debug以添加调试信息
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# 查找所有源文件
file(GLOB SOURCES "*.c")

# 指定包含目录
include_directories("C:/msys64/mingw64/include")

# 添加要编译的可执行文件
add_executable(FlexDemo ${SOURCES})

# 如果需要链接到其他库，使用target_link_libraries
# 这里假设你不需要链接libfl.a，因为我们不使用yywrap或者已经提供了yywrap的实现
# 如果需要链接libfl.a或其他库，可以这样添加：
target_link_libraries(FlexDemo "C:/msys64/usr/lib/libfl.a")
