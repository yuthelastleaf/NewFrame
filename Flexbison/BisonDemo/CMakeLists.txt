cmake_minimum_required(VERSION 3.0)
project(FlexDemoProject)

set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")

# 查找Flex和Bison
find_package(BISON)
find_package(FLEX)

# 定义Bison和Flex的输入和输出文件
set(BISON_INPUT ${CMAKE_SOURCE_DIR}/fb1-5.y)
set(FLEX_INPUT ${CMAKE_SOURCE_DIR}/fb1-5.l)
set(BISON_OUTPUT ${CMAKE_BINARY_DIR}/fb1-5.tab.c)
set(FLEX_OUTPUT ${CMAKE_BINARY_DIR}/lex.yy.c)

add_custom_command(
  OUTPUT ${BISON_OUTPUT}
  COMMAND ${BISON_EXECUTABLE} -d ${BISON_INPUT} -o ${BISON_OUTPUT}
  DEPENDS ${BISON_INPUT}
  COMMENT "Generating fb1-5.tab.c with bison"
)

# 使用add_custom_command调用Flex
add_custom_command(
  OUTPUT ${FLEX_OUTPUT}
  COMMAND ${FLEX_EXECUTABLE} "${FLEX_INPUT}"
  DEPENDS ${FLEX_INPUT}
  COMMENT "Generating lex.yy.c with flex"
)


# 添加定义，如果你不需要yywrap可以定义此宏
add_definitions(-DYY_NO_UNISTD_H)

# 设置构建类型为Debug以添加调试信息
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# 指定包含目录
include_directories("C:/msys64/mingw64/include")

# 添加要编译的可执行文件
add_executable(FlexDemo ${BISON_OUTPUT} ${FLEX_OUTPUT})

# 如果需要链接到其他库，使用target_link_libraries
# 这里假设你不需要链接libfl.a，因为我们不使用yywrap或者已经提供了yywrap的实现
# 如果需要链接libfl.a或其他库，可以这样添加：
target_link_libraries(FlexDemo "C:/msys64/usr/lib/libfl.a")
