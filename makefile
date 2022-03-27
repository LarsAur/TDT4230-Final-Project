# TODO: https://stackoverflow.com/questions/714100/os-detecting-makefile
.PHONY: run build clean

CMAKE_ARGS = -B ./build -S .
ifeq ($(OS), Windows_NT)
	CMAKE_ARGS += -G "MinGW Makefiles"
	EXECUTABLE = PortalProject.exe
else
	EXECUTABLE = ./PortalProject
endif

run:
	cd ./build && make && $(EXECUTABLE)

debug:
	cd ./build && make && gdb $(EXECUTABLE)

build:
	cmake $(CMAKE_ARGS)

buildDebug:
	cmake $(CMAKE_ARGS) -DCMAKE_BUILD_TYPE=Debug

clean:
	-del /q /s build\*
	-rmdir /q /s build
	-mkdir build && cd build && copy NUL .gitkeep