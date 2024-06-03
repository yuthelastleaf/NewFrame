cd %~dp0
rm -r build
mkdir build
cd build
cmake ../
cmake --build .