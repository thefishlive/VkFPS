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
#include "g_device.h"
#include "GLFW/glfw3.h"

GraphicsDevice::GraphicsDevice()
{
	// Create Vulkan Instance
	vk::ApplicationInfo application_info(
		"Vulkan FPS",
		VK_MAKE_VERSION(0, 1, 0),
		"Vulkan Basics",
		VK_MAKE_VERSION(0, 1, 0)
	);

	std::vector<const char *> instance_layers;
	std::vector<const char *> instance_extensions;

#if defined(ENABLE_VK_LAYER_VALIDATION) || defined(ENABLE_VK_LAYER_ASSISTANT)
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	uint32_t glfw_ext_count;
	const char ** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

	for (uint32_t i = 0; i < glfw_ext_count; i++)
	{
		instance_extensions.push_back(glfw_exts[i]);
	}

#ifdef ENABLE_VK_LAYER_VALIDATION
	instance_layers.push_back("VK_LAYER_LUNARG_core_validation");
#endif
#ifdef ENABLE_VK_LAYER_ASSISTANT
	instance_layers.push_back("VK_LAYER_LUNARG_assistant_layer");
#endif
#ifdef ENABLE_VK_LAYER_MONITOR
	instance_layers.push_back("VK_LAYER_LUNARG_monitor");
#endif

	vk::InstanceCreateInfo create_info(
		vk::InstanceCreateFlags(0),
		&application_info,
		(uint32_t) instance_layers.size(), instance_layers.data(),
		(uint32_t) instance_extensions.size(), instance_extensions.data()
	);

	instance = vk::createInstanceUnique(create_info);

	// Select physical device
	QueueFamilyIndicies queue_data;
	physical_deivce = select_physical_device(queue_data);

	// Create Logical Device
	std::vector<vk::DeviceQueueCreateInfo> queues;
	std::array<float, 1> queue_priorities;
	std::vector<const char *> device_extensions;
	vk::PhysicalDeviceFeatures physical_device_feature;

	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	queue_priorities[0] = 1.0f;

	queues.push_back(vk::DeviceQueueCreateInfo(
		vk::DeviceQueueCreateFlags(0),
		queue_data.graphics_queue,
		(uint32_t) queue_priorities.size(), queue_priorities.data()
	));
	
	vk::DeviceCreateInfo device_create_info(
		vk::DeviceCreateFlags(0),
		(uint32_t) queues.size(), queues.data(),
		0, nullptr,
		(uint32_t) device_extensions.size(), device_extensions.data(),
		&physical_device_feature
	);

	device = physical_deivce.createDeviceUnique(device_create_info, nullptr);

	graphics_queue = GraphicsQueue(device->getQueue(queue_data.graphics_queue, 0));
}

GraphicsDevice::~GraphicsDevice()
{
}

bool GraphicsDevice::is_device_suitable(vk::PhysicalDevice physical_device, QueueFamilyIndicies & queue_data)
{
	auto properties = physical_device.getProperties();
	auto queue_families = physical_device.getQueueFamilyProperties();

	unsigned i = 0;
	for (auto const& queue_family : queue_families)
	{
		if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queue_data.graphics_queue = i;
		}

		i++;
	}

	return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
		queue_data.is_complete();
}

vk::PhysicalDevice GraphicsDevice::select_physical_device(QueueFamilyIndicies & queue_data)
{
	auto physical_devices = this->instance->enumeratePhysicalDevices();

	for (auto const& physical_device : physical_devices) 
	{
		if (this->is_device_suitable(physical_device, queue_data))
		{
			return physical_device;
		}
	}

	throw std::exception("Cannot find suitable vulkan device");
}


