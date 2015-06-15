# knet
cross platform, multithreading net library

# implements
support IOCP, select, epoll.

# build - windows
check directory win-proj

# build - linux 
cmake CMakeLists.txt   
make   

# run example
open examples/example_config.h   
set the macro value as your wish and rebuild   
type in the shell: ./bin/examples

# run test
windows:    
    test_client-vs2010.exe -ip 192.168.0.27 -port 8000 -n 500    
	test_server-vs2010.exe -ip 192.168.0.27 -port 8000 -w 4    
	
linux:    
	./test_client -ip 192.168.0.27 -port 8000 -n 500    
	./test_server -ip 192.168.0.27 -port 8000 -w 4    

have fun!
