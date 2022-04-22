# TDT4230-Final-Project
This was my delivery for the final project in TDT4230 - Graphics and Visualization at NTNU.
I implemented Portals in OpenGL using stencil buffers. Demo video: https://youtu.be/noxH2141WDc

When playing, portals can be placed by looking at the walls and clicking the mouse buttons. The
player can move using the `WASD`, `left shift` and `space`.

![image](https://user-images.githubusercontent.com/3136092/164721484-287be8f7-cdf5-4570-929a-e80aa6211bce.png)

# Setup

* Clone the project: `git clone --recursive https://github.com/LarsAur/TDT4230-Final-Project.git`  
* Run cmake: `make build` or `cmake -B ./build -S`  
* Compile and run: `make run`  

 *Note: When compiling for windows `make build` assumes the usage of MinGW Makefiles* 

# Libraries

* GLFW (https://github.com/glfw/glfw) For window creation and window management  
* GLM (https://github.com/g-truc/glm) For vector/matrix/quaternion operations  
* STB (https://github.com/nothings/stb) For image decoding/loading  
* glad (https://github.com/Dav1dde/glad) For accessing OpenGL functions    
* Perlin noise (https://github.com/Reputeless/PerlinNoise) For generating Perlin noise  

# Resources

* How to create oblique view frustum: http://www.terathon.com/lengyel/Lengyel-Oblique.pdf

