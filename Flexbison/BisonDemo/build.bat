cd %~dp0
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER="C:\msys64\mingw64\bin\gcc.exe" -DCMAKE_CXX_COMPILER="C:\msys64\mingw64\bin\g++.exe" ../
cmake --build .s