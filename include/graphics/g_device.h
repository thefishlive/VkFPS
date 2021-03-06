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
#include "g_transfer_context.h"
#include "g_window.h"

struct QueueFamilyIndicies
{
	uint32_t graphics_queue = -1;
	uint32_t present_queue = -1;
    uint32_t transfer_queue = -1;

    bool is_complete() const
    {
		return graphics_queue != -1 && present_queue != -1 && transfer_queue != -1;
	}
};

class GraphicsDevice
{
public:
	GraphicsDevice(std::shared_ptr<GraphicsWindow>& window);
	GraphicsDevice(GraphicsDevice & device) = delete;
	~GraphicsDevice();

	vk::Instance instance;
	vk::PhysicalDevice physical_deivce;
	vk::Device device;

	vk::Semaphore create_semaphore() const;

    std::unique_ptr<GraphicsTransferContext> transfer_context;

    std::shared_ptr<GraphicsQueue> graphics_queue;
    std::shared_ptr<GraphicsQueue> present_queue;
    std::shared_ptr<GraphicsQueue> transfer_queue;

private:
	vk::DebugUtilsMessengerEXT debug_report_callback;

	static bool is_device_suitable(::vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data);
	vk::PhysicalDevice select_physical_device(vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data) const;
};
