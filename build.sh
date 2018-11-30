if [ ! -d ./bin ]; then
    mkdir bin/cpp_tpl 
fi
if [ ! -d ./lib ]; then
    mkdir lib
fi

use_ipv6=0
for arg in $@; do
	if [ "$arg" = "ipv6" ] || [ "$arg" = "IPV6" ]; then
		use_ipv6=1
	fi
done

if [ $use_ipv6 -eq 1 ]; then
	cmake -DCMAKE_BUILD_TYPE=IPV6 CMakeLists.txt
else
	cmake CMakeLists.txt
fi

make install
