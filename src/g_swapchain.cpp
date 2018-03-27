
#include <stdint.h>

#include "g_swapchain.h"
#include "g_window.h"

GraphicsSwapchain::GraphicsSwapchain(GraphicsWindow & window, GraphicsDevice & device)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = device.physical_deivce.getSurfaceCapabilitiesKHR(window.surface);

	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0)
	{
		std::min(image_count, surface_capabilities.maxImageCount);
	}

	vk::SurfaceFormatKHR image_format = this->select_image_format(device.physical_deivce.getSurfaceFormatsKHR(window.surface));
	vk::PresentModeKHR present_mode = this->select_present_mode(device.physical_deivce.getSurfacePresentModesKHR(window.surface));
	vk::Extent2D image_extent = this->select_swap_extent(surface_capabilities);

	vk::SharingMode image_sharing_mode = vk::SharingMode::eExclusive;
	std::vector<uint32_t> queue_families;

	vk::SwapchainCreateInfoKHR swapchain_create_info(
		vk::SwapchainCreateFlagsKHR(0),
		window.surface,
		image_count,
		image_format.format,
		image_format.colorSpace,
		image_extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		image_sharing_mode,
		(uint32_t)queue_families.size(), queue_families.data(),
		vk::SurfaceTransformFlagBitsKHR::eIdentity,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		present_mode
	);

	device.device->createSwapchainKHRUnique(swapchain_create_info);
}

GraphicsSwapchain::~GraphicsSwapchain()
{
	
}


uint32_t GraphicsSwapchain::aquire_image(vk::Device device, vk::Semaphore aquire_semaphore)
{/*
	uint32_t index;
	device.acquireNextImageKHR(vk::SwapchainKHR(), std::numeric_limits<uint32_t>::max(), aquire_semaphore, VK_NULL_HANDLE, &index);*/
	return -1;
}

void GraphicsSwapchain::present_image(vk::Device device, uint32_t image_index, vk::Semaphore wait_semaphore)
{
}

vk::SurfaceFormatKHR GraphicsSwapchain::select_image_format(std::vector<vk::SurfaceFormatKHR> image_formats) const
{
	if (image_formats.size() == 0)
	{
		throw std::exception("No formats supported");
	}

	if (image_formats.size() == 1 && image_formats[0].format == vk::Format::eUndefined)
	{
		return vk::SurfaceFormatKHR({ vk::Format::eB8G8R8A8Snorm, vk::ColorSpaceKHR::eSrgbNonlinear });
	}

	for (const auto& format : image_formats) {
		if (format.format == vk::Format::eB8G8R8A8Snorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}

	return image_formats[0];
}

vk::PresentModeKHR GraphicsSwapchain::select_present_mode(std::vector<vk::PresentModeKHR> present_modes) const
{
	vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

	for (const auto& mode : present_modes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			return mode;
		}

		if (mode == vk::PresentModeKHR::eImmediate) {
			best_mode = mode;
		}
	}

	return best_mode;
}

vk::Extent2D GraphicsSwapchain::select_swap_extent(vk::SurfaceCapabilitiesKHR surface_capabilities) const
{
	// Return the current extent if it is set
	if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surface_capabilities.currentExtent;
	}

	vk::Extent2D actual_extent = vk::Extent2D(800, 800);
	actual_extent.width = std::max(surface_capabilities.minImageExtent.width, std::min(actual_extent.width, surface_capabilities.maxImageExtent.width));
	actual_extent.height = std::max(surface_capabilities.minImageExtent.height, std::min(actual_extent.height, surface_capabilities.maxImageExtent.height));
	return actual_extent;
}
