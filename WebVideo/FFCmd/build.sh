rm -f parser.tab.h
rm -f parser.tab.c
rm -f lex.yy.c
# 创建并进入构建目录
mkdir -p build
cd build
# 使用 CMake 配置项目
cmake ../
# 编译项目
make