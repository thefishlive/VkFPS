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

#include <memory>
#include <vulkan/vulkan.hpp>

#include "g_queue.h"

struct QueueFamilyIndicies
{
	uint32_t graphics_queue = -1;
	uint32_t present_queue = -1;

	bool is_complete() const
	{
		return graphics_queue != -1 /*&& present_queue != -1*/;
	}
};

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

private:
	vk::UniqueInstance instance;
	vk::PhysicalDevice physical_deivce;
	vk::UniqueDevice device;

	GraphicsQueue graphics_queue;
	GraphicsQueue present_queue;

	static bool is_device_suitable(::vk::PhysicalDevice physical_device, QueueFamilyIndicies & queue_data);
	vk::PhysicalDevice select_physical_device(QueueFamilyIndicies & queue_data);
};
