# TODO: https://stackoverflow.com/questions/714100/os-detecting-makefile
.PHONY: run build clean

run:
	cd ./build && make && PortalProject.exe

debug:
	cd ./build && make && gdb ./PortalProject.exe

build:
	cmake -G "MinGW Makefiles" -B ./build -S .

buildDebug:
	cmake -G "MinGW Makefiles" -B ./build -S . -DCMAKE_BUILD_TYPE=Debug

build-linux:
	cmake -B ./build -S .

clean:
	-del /q /s build\*
	-rmdir /q /s build
	-mkdir build && cd build && copy NUL .gitkeep