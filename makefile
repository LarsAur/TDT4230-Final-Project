
.PHONY: run build clean

run:
	cd ./build && make && PortalProject.exe

build:
	cmake -G "MinGW Makefiles" -B ./build -S .

clean:
	-del /q /s build\*
	-rmdir /q /s build\*
	-mkdir build && cd build && copy NUL .gitkeep