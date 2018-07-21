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

#include "g_device.h"
#include "g_window.h"

class GraphicsSwapchain
{
public:
	GraphicsSwapchain(std::shared_ptr<GraphicsWindow>& window, std::shared_ptr<GraphicsDevice>& device);
	~GraphicsSwapchain();

	uint32_t aquire_image(vk::Device device, vk::Semaphore aquire_semaphore) const;
	void present_image(vk::Device device, uint32_t image_index, vk::Semaphore wait_semaphore) const;

	uint32_t get_image_count() const { return (uint32_t) images.size(); }
	vk::ImageView get_image_view(uint32_t i) const { return image_views[i]; }
	vk::Extent2D get_extent() const { return swapchain_extent; }
	vk::SurfaceFormatKHR get_swapchain_format() const { return swapchain_format; }

private:
	std::shared_ptr<GraphicsDevice>& device;
	vk::SwapchainKHR swapchain;

	std::vector<vk::Image> images;
	std::vector<vk::ImageView> image_views;

	vk::SurfaceFormatKHR swapchain_format;
	vk::Extent2D swapchain_extent;
	vk::Queue present_queue;

	static vk::SurfaceFormatKHR select_image_format(std::vector<vk::SurfaceFormatKHR> image_formats);
	static vk::PresentModeKHR select_present_mode(std::vector<vk::PresentModeKHR> present_modes);
	vk::Extent2D select_swap_extent(vk::SurfaceCapabilitiesKHR surface_capabilities) const;
};
