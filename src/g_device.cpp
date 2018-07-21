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

#include <iostream>
#include <set>

#include <GLFW/glfw3.h>

#include "g_fence.h"
#include "g_window.h"
#include "u_debug.h"
#include "u_defines.h"

VkResult vkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger
)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (!func)
	{
		return VK_SUCCESS;
	}
	return func(instance, pCreateInfo, pAllocator, pMessenger);
}

void vkDestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator
)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (!func)
	{
		return;
	}
	return func(instance, messenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT            severity,
    VkDebugUtilsMessageTypeFlagsEXT                   msgType,
	const VkDebugUtilsMessengerCallbackDataEXT       *pCallbackData,
	void* userData)
{
	if (BITMASK_HAS(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ||
		BITMASK_HAS(severity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
	{
		LOG_ERROR("%s:%s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else
	{
		LOG_INFO("%s:%s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}

	return VK_FALSE;
}

GraphicsDevice::GraphicsDevice(std::shared_ptr<GraphicsWindow>& window)
{
	// Create Vulkan Instance
	vk::ApplicationInfo application_info(
		"Vulkan FPS",
		VK_MAKE_VERSION(0, 1, 0),
		"Vulkan Basics",
		VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_0
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
	instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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

	instance = vk::createInstance(create_info);

#if defined(ENABLE_VK_LAYER_VALIDATION) || defined(ENABLE_VK_LAYER_ASSISTANT)
	debug_report_callback = instance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(0),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        &debug_callback
	));
#endif

	vk::SurfaceKHR surface = window->create_surface(instance);

	// Select physical device
	QueueFamilyIndicies queue_data;
	physical_deivce = select_physical_device(surface, queue_data);

	// Create Logical Device
	std::set<uint32_t> queue_indicies = {queue_data.graphics_queue, queue_data.present_queue, queue_data.transfer_queue};
	std::vector<vk::DeviceQueueCreateInfo> queues;
	std::array<float, 1> queue_priorities;
	std::vector<const char *> device_extensions;

	vk::PhysicalDeviceFeatures physical_device_feature;
	physical_device_feature.setIndependentBlend(VK_TRUE);

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

	device = physical_deivce.createDevice(device_create_info, nullptr);

    graphics_queue = std::make_shared<GraphicsQueue>(this, queue_data.graphics_queue);
    present_queue = std::make_shared<GraphicsQueue>(this, queue_data.graphics_queue);
    transfer_queue = std::make_shared<GraphicsQueue>(this, queue_data.graphics_queue);

    transfer_context = std::make_unique<GraphicsTransferContext>(this);
}

GraphicsDevice::~GraphicsDevice()
{
	device.destroy();

	instance.destroyDebugUtilsMessengerEXT(debug_report_callback);
	instance.destroy();
}

vk::Semaphore GraphicsDevice::create_semaphore() const
{
	return device.createSemaphore(vk::SemaphoreCreateInfo(
		vk::SemaphoreCreateFlags(0)
	));
}

bool GraphicsDevice::is_device_suitable(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data)
{
	auto properties = physical_device.getProperties();
	auto features = physical_device.getFeatures();
	auto queue_families = physical_device.getQueueFamilyProperties();

	unsigned i = 0;
	for (auto const& queue_family : queue_families)
	{
        if (queue_family.queueCount <= 0)
        {
            continue;
        }

        /* Check for graphics support */
		if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queue_data.graphics_queue = i;
		}

        /* Check for presentation support */
		if (physical_device.getSurfaceSupportKHR(i, surface))
		{
			queue_data.present_queue = i;
		}

        /* Check for dedicated transfer queue */
        if (queue_family.queueFlags & vk::QueueFlagBits::eTransfer && !(queue_family.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            queue_data.transfer_queue = i;
        }

		i++;
	}

    /* If there is no dedicated transfer queue, fallback to graphics queue */
    if (queue_data.transfer_queue == -1)
    {
        queue_data.transfer_queue = queue_data.graphics_queue;
    }

	return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
		features.independentBlend &&
		queue_data.is_complete();
}

vk::PhysicalDevice GraphicsDevice::select_physical_device(vk::SurfaceKHR surface, QueueFamilyIndicies & queue_data) const
{
	// TODO Better grading of devices
	auto physical_devices = this->instance.enumeratePhysicalDevices();

	for (auto const& physical_device : physical_devices) 
	{
		if (this->is_device_suitable(physical_device, surface, queue_data))
		{
			return physical_device;
		}
	}

	throw std::exception("Cannot find suitable vulkan device");
}


