cmake_minimum_required(VERSION 3.10)
project(FFCmd)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 找到 Flex 和 Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# 指定 Flex 和 Bison 文件
set(LEXER_FILE lexer.l)
set(PARSER_FILE parser.y)

# 生成 Bison 文件
BISON_TARGET(Parser ${PARSER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)

# 生成 Flex 文件
FLEX_TARGET(Lexer ${LEXER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

# 添加 Flex/Bison 依赖关系
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

# Set this to your FFmpeg include directory
set(FFMPEG_INCLUDE_DIR ../../ThirdLib/ffmpeg_install/include)
# Set this to your FFmpeg lib directory
set(FFMPEG_LIB_DIR ${CMAKE_SOURCE_DIR}/../../ThirdLib/ffmpeg_install/bin)
set(FFMPEG_DEV_ROOT ../../ThirdLib/ffmpeg_install)
# Set x264 library directorys
set(X264_LIB_DIR ${CMAKE_SOURCE_DIR}/../../ThirdLib/x264_install/bin)

include_directories(${FFMPEG_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${FFMPEG_DEV_ROOT}/bin)
# config flex library directory, it can be custom
link_directories(C:\\msys64\\usr\\lib)

add_executable(ffcmd
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
    handle_param.cpp
    FFCmd.cpp)

# Specify the static libraries from FFmpeg you're linking against
target_link_libraries(ffcmd
    avformat
    avcodec
    avutil
    avfilter
    swresample
    fl
    # Add any other FFmpeg libraries you need here
)

# 在 Windows 上，复制 FFmpeg 的 DLLs 到可执行文件同一目录
if(WIN32)
    add_custom_command(TARGET ffcmd POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FFMPEG_LIB_DIR}/avcodec-61.dll"
        "${FFMPEG_LIB_DIR}/avformat-61.dll"
        "${FFMPEG_LIB_DIR}/avutil-59.dll"
        "${FFMPEG_LIB_DIR}/avfilter-10.dll"
        "${FFMPEG_LIB_DIR}/postproc-58.dll"
        "${FFMPEG_LIB_DIR}/swscale-8.dll"
        "${X264_LIB_DIR}/libx264-164.dll"
        "${FFMPEG_LIB_DIR}/swresample-5.dll"
        $<TARGET_FILE_DIR:ffcmd>)
endif()

# If you're using MSVC, you might need to suppress some warnings or set specific compiler options
if(MSVC)
    # 仅对 MSVC 编译器
    target_compile_options(ffcmd PRIVATE /W3)
else()
    # 对 GCC 或 Clang 等其他编译器
    target_compile_options(ffcmd PRIVATE -Wall -Wextra)
endif()
