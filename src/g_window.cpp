/******************************************************************************
* Copyright 2017 James Fitzpatrick <james_fitzpatrick@outlook.com>           *
*                                                                            *
* Permission is hereby granted, free of charge, to any person obtaining a    *
* copy of this software and associated documentation files (the "Software"), *
* to deal in the Software without restriction, including without limitation  *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
* and/or sell copies of the Software, and to permit persons to whom the      *
* Software is furnished to do so, subject to the following conditions:       *
*                                                                            *
* The above copyright notice and this permission notice shall be included in *
* all copies or substantial portions of the Software.                        *
*                                                                            *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
* DEALINGS IN THE SOFTWARE.                                                  *
******************************************************************************/

#include "g_window.h"

static void glfw_error_callback(int error, const char *description)
{
	throw std::exception(description);
}

GraphicsWindow::GraphicsWindow(std::string window_name, uint32_t width, uint32_t height)
{
	glfwInit();

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwSetErrorCallback(&glfw_error_callback);

	GLFWwindow *window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	
	this->window = std::shared_ptr<GLFWwindow>(window);
	this->window_size = vk::Extent2D(width, height);
}

GraphicsWindow::~GraphicsWindow()
{
	glfwTerminate();
}

vk::SurfaceKHR GraphicsWindow::create_surface(vk::Instance instance)
{
	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface((VkInstance)instance, this->window.get(), nullptr, &surface);
	if (result < 0)
	{
		std::exception("Error creating window surface");
	}

	this->surface = vk::SurfaceKHR(surface);

	return this->surface;
}

GraphicsWindow::GraphicsWindow(GraphicsWindow& window)
	: surface(window.surface), 
		window(window.window), 
		window_size(window.window_size)
{

}
