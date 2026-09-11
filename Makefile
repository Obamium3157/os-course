all: linux windows

linux:
	mkdir -p build-linux && cd build-linux && cmake .. && make

windows:
	mkdir -p build-windows && cd build-windows && cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake && make

clean:
	rm -rf build-*


l1t1: windows
	./run build-windows/lab1/task1/SysInfoWin.exe
l1t2: linux
	./build-linux/lab1/task2/SysInfoLinux
l1t3: all
	./run build-windows/lab1/task3/SysInfoCrossplatform.exe
	./build-linux/lab1/task3/SysInfoCrossplatform