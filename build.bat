mkdir build  
cd build
cmake -G "MinGW Makefiles"  ..
make -j 4
cp handwrite.exe ../output
