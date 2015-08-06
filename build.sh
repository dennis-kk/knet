if [ ! -d ./bin ]; then
    mkdir bin
    mkdir bin/cpp_tpl 
fi
if [ ! -d ./lib ]; then
    mkdir lib
fi
cmake CMakeLists.txt
make
