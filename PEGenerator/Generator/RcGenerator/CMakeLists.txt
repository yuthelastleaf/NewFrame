cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(MyProject VERSION 1.0)

# 添加UNICODE定义
add_definitions(-DUNICODE -D_UNICODE)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定源文件
set(SOURCE_FILES main.cpp)

# 如果是Windows平台，添加资源文件的编译
if(WIN32)
    # 指定资源文件
    set(RESOURCE_FILE resources.rc)
    
    # 添加资源文件到源文件列表
    list(APPEND SOURCE_FILES ${RESOURCE_FILE})
endif()

# 添加可执行文件
add_executable(MyApplication ${SOURCE_FILES})

# 如果你的项目需要，添加其他链接库
# target_link_libraries(MyApplication ...)
