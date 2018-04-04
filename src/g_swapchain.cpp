
#include <stdint.h>

#include "g_swapchain.h"
#include "g_window.h"

GraphicsSwapchain::GraphicsSwapchain(GraphicsWindow & window, std::shared_ptr<GraphicsDevice>& device)
	: device(device)
{
	vk::SurfaceCapabilitiesKHR surface_capabilities = device->physical_deivce.getSurfaceCapabilitiesKHR(window.surface);

	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0)
	{
		std::min(image_count, surface_capabilities.maxImageCount);
	}

	vk::SurfaceFormatKHR image_format = this->select_image_format(device->physical_deivce.getSurfaceFormatsKHR(window.surface));
	vk::PresentModeKHR present_mode = this->select_present_mode(device->physical_deivce.getSurfacePresentModesKHR(window.surface));
	vk::Extent2D image_extent = this->select_swap_extent(surface_capabilities);

	// TODO allow for combined graphics/present queues (Sharing Mode Concurrent)
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

	this->swapchain = device->device.createSwapchainKHR(swapchain_create_info);
	this->swapchain_format = image_format;
	this->swapchain_extent = image_extent;

	this->images = device->device.getSwapchainImagesKHR(this->swapchain);
	
	for (const auto & image : this->images)
	{
		vk::ImageViewCreateInfo view_create_info(
			vk::ImageViewCreateFlags(0),
			image,
			vk::ImageViewType::e2D,
			image_format.format,
			vk::ComponentMapping(
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			),
			vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1,
				0, 1
			)
		);

		this->image_views.push_back(device->device.createImageView(view_create_info));
	}

	this->present_queue = device->graphics_queue.queue;
}

GraphicsSwapchain::~GraphicsSwapchain()
{
	for (const auto &view : image_views)
	{
		device->device.destroyImageView(view);
	}

	device->device.destroySwapchainKHR(swapchain);
}


uint32_t GraphicsSwapchain::aquire_image(vk::Device device, vk::Semaphore aquire_semaphore) const
{
	return device.acquireNextImageKHR(this->swapchain, std::numeric_limits<uint32_t>::max(), aquire_semaphore, vk::Fence()).value;
}

void GraphicsSwapchain::present_image(vk::Device device, uint32_t image_index, vk::Semaphore wait_semaphore) const
{
	std::vector<vk::Semaphore> wait_semaphores = { wait_semaphore };
	std::vector<vk::SwapchainKHR> swapchains = { swapchain };
	std::vector<uint32_t> image_indicies = { image_index };

	vk::PresentInfoKHR present_info(
		(uint32_t) wait_semaphores.size(), wait_semaphores.data(),
		(uint32_t) swapchains.size(), swapchains.data(), image_indicies.data()
	);

	present_queue.presentKHR(present_info);
}

vk::SurfaceFormatKHR GraphicsSwapchain::select_image_format(std::vector<vk::SurfaceFormatKHR> image_formats) const
{
	assert(image_formats.size() > 0);

	if (image_formats.size() == 1 && image_formats[0].format == vk::Format::eUndefined)
	{
		return vk::SurfaceFormatKHR({ vk::Format::eB8G8R8A8Snorm, vk::ColorSpaceKHR::eSrgbNonlinear });
	}

	// TODO select better image format
	for (const auto& format : image_formats) {
		if (format.format == vk::Format::eB8G8R8A8Snorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}

	return image_formats[0];
}

vk::PresentModeKHR GraphicsSwapchain::select_present_mode(std::vector<vk::PresentModeKHR> present_modes) const
{
	assert(present_modes.size() > 0);

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
