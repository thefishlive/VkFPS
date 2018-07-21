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
#pragma once

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class GraphicsWindow
{
public:
	GraphicsWindow(std::string window_name, uint32_t width, uint32_t height);
    GraphicsWindow(GraphicsWindow & window) = delete;
	~GraphicsWindow();

	vk::SurfaceKHR create_surface(vk::Instance instance);

    bool should_close() const;
	void poll_events() const { glfwPollEvents(); }

	vk::Extent2D get_window_size() const { return window_size; }
    void close() const;

    int get_key_state(int key) const;
    glm::vec2 get_mouse_delta();

    vk::SurfaceKHR surface;
private:
	std::shared_ptr<GLFWwindow> window;

	vk::Extent2D window_size;
    glm::vec2 last_mouse_pos;
};
