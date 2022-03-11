#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <string>
#include <iostream>

static void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void errorCallback(int error, const char *description)
{
	if(error != GLFW_INVALID_ENUM) // Suppressing invalid key errors (and sadly other invalid enum errors)
    fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
}

typedef struct MousePosition
{
	double x;
	double y;
} MousePosition;

class Window
{
private:
	GLFWwindow *mWindow;
	int mWidth;
	int mHeight;

	bool mPrevIsKeyDown[GLFW_KEY_LAST] = {false};
	bool mPrevIsMouseDown[GLFW_KEY_LAST] = {false};
	bool mIsKeyPressed[GLFW_KEY_LAST] = {false};
	bool mIsMousePressed[GLFW_MOUSE_BUTTON_LAST] = {false};
	MousePosition mMousePosition = {0, 0};
	MousePosition mMouseDelta = {0, 0};

public:
	Window(int width, int height, std::string title)
	{
		mWidth = width;
		mHeight = height;

		if (!glfwInit())
		{
			std::cout << "Failed to init GLFW" << std::endl;
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwSetErrorCallback(errorCallback);

		mWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

		if (!mWindow)
		{
			std::cout << "Failed to initialize window" << std::endl;
			glfwTerminate();
		}

		glfwMakeContextCurrent(mWindow);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glfwSwapInterval(1);
		
		// Add window callbacks
		glfwSetFramebufferSizeCallback(mWindow, framebufferSizeCallback);
	}

	int getWidth()
	{
		glfwGetFramebufferSize(mWindow, &mWidth, NULL);
		return mWidth;
	}

	int getHeight()
	{
		glfwGetFramebufferSize(mWindow, NULL, &mHeight);
		return mHeight;
	}

	bool shouldClose()
	{
		return glfwWindowShouldClose(mWindow);
	}

	void close()
	{
		glfwSetWindowShouldClose(mWindow, 1);
	}

	void swapBuffers()
	{
		glfwSwapBuffers(mWindow);
	}

	void destroy()
	{
		glfwDestroyWindow(mWindow);
	}

	void disableCursor()
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void enableCursor()
	{
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// Input handling

	bool isKeyDown(int key)
	{
		return glfwGetKey(mWindow, key) == GLFW_PRESS;
	}

	bool isKeyPressed(int key)
	{
		if(key < 0 || key > GLFW_KEY_LAST) return false;
		return mIsKeyPressed[key];
	}

	bool isMouseButtonDown(int button)
	{
		return glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
	}

	bool isMouseButtonPressed(int button)
	{
		if(button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
		return mIsMousePressed[button];
	}

	MousePosition getMousePosition()
	{
		return mMousePosition;
	}

	MousePosition getMouseDelta()
	{
		return mMouseDelta;
	}

	void updateInput()
	{
		glfwPollEvents();

		for(int i = 0; i < GLFW_KEY_LAST; i++)
		{
			mIsKeyPressed[i] = isKeyDown(i) && !mPrevIsKeyDown[i];
			mPrevIsKeyDown[i] = isKeyDown(i);
		}

		for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
		{
			mIsMousePressed[i] = isMouseButtonDown(i) && !mPrevIsMouseDown[i];
			mPrevIsMouseDown[i] = isMouseButtonDown(i);
		}

		double xpos, ypos;
		glfwGetCursorPos(mWindow, &xpos, &ypos);

		mMouseDelta = {xpos - mMousePosition.x, ypos - mMousePosition.y};
		mMousePosition = {xpos, ypos};
	}
};
