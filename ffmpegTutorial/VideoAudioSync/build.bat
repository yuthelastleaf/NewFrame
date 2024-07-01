cd %~dp0
rmdir /s /q build
mkdir build
cd build
cmake ../
cmake --build .