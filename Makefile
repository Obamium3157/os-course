all: linux windows

linux:
	mkdir -p build-linux && cd build-linux && cmake .. && make

windows:
	mkdir -p build-windows && cd build-windows && cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake && make

clean:
	rm -rf build-*