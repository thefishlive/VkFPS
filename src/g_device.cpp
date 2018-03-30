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
#include "g_window.h"
#include <iostream>
#include <set>

VkResult vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT * pCreateInfo,
	const VkAllocationCallbacks * pAllocator,
	VkDebugReportCallbackEXT * pCallback
)
{
	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	return func(instance, pCreateInfo, pAllocator, pCallback);
}

void vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks * pAllocator
)
{
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	return func(instance, callback, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::cerr << "Validation Error: " << layerPrefix << ": " << msg << std::endl;
	return VK_FALSE;
}

GraphicsDevice::GraphicsDevice(GraphicsWindow & window)
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

	uint32_t glfw_ext_count;
	const char ** glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_ext_count);

	for (uint32_t i = 0; i < glfw_ext_count; i++)
	{
		instance_extensions.push_back(glfw_exts[i]);
	}

#if defined(ENABLE_VK_LAYER_VALIDATION) || defined(ENABLE_VK_LAYER_ASSISTANT)
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

#ifdef ENABLE_VK_LAYER_VALIDATION
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
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

	debug_report_callback = instance.get().createDebugReportCallbackEXTUnique(vk::DebugReportCallbackCreateInfoEXT(
		vk::DebugReportFlagBitsEXT::eError |
		vk::DebugReportFlagBitsEXT::eWarning |
		vk::DebugReportFlagBitsEXT::ePerformanceWarning |
		vk::DebugReportFlagBitsEXT::eInformation,
		&debug_callback
	));

	vk::SurfaceKHR surface = window.create_surface(instance.get());

	// Select physical device
	QueueFamilyIndicies queue_data;
	physical_deivce = select_physical_device(surface, queue_data);

	// Create Logical Device
	std::set<uint32_t> queue_indicies = {queue_data.graphics_queue, queue_data.present_queue};
	std::vector<vk::DeviceQueueCreateInfo> queues;
	std::array<float, 1> queue_priorities;
	std::vector<const char *> device_extensions;
	vk::PhysicalDeviceFeatures physical_device_feature;

	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	queue_priorities[0] = 1.0f;

	for (const auto & index : queue_indicies)
	{
		queues.push_back(vk::DeviceQueueCreateInfo(
			vk::DeviceQueueCreateFlags(0),
			index,
			(uint32_t)queue_priorities.size(), queue_priorities.data()
		));
	}

	vk::DeviceCreateInfo device_create_info(
		vk::DeviceCreateFlags(0),
		(uint32_t) queues.size(), queues.data(),
		0, nullptr,
		(uint32_t) device_extensions.size(), device_extensions.data(),
		&physical_device_feature
	);

	device = physical_deivce.createDeviceUnique(device_create_info, nullptr);

	graphics_queue = GraphicsQueue(device.get(), queue_data.graphics_queue);
	present_queue = GraphicsQueue(device.get(), queue_data.present_queue);
}

GraphicsDevice::~GraphicsDevice()
{
}

vk::UniqueSemaphore GraphicsDevice::create_semaphore()
{
	return device.get().createSemaphoreUnique(vk::SemaphoreCreateInfo(
		vk::SemaphoreCreateFlags(0)
	));
}

bool GraphicsDevice::is_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data) const
{
	auto properties = physical_device.getProperties();
	auto queue_families = physical_device.getQueueFamilyProperties();

	unsigned i = 0;
	for (auto const& queue_family : queue_families)
	{
		if (queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queue_data.graphics_queue = i;
		}

		if (queue_family.queueCount > 0 && physical_device.getSurfaceSupportKHR(i, surface))
		{
			queue_data.present_queue = i;
		}

		i++;
	}

	// TODO remove dependency on discrete gpus
	return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
		queue_data.is_complete();
}

vk::PhysicalDevice GraphicsDevice::select_physical_device(vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data) const
{
	// TODO Better grading of devices
	auto physical_devices = this->instance->enumeratePhysicalDevices();

	for (auto const& physical_device : physical_devices) 
	{
		if (this->is_device_suitable(physical_device, surface, queue_data))
		{
			return physical_device;
		}
	}

	throw std::exception("Cannot find suitable vulkan device");
}


