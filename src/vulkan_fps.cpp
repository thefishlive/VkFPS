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

#include <iostream>

#include "g_device.h"
#include "g_devmem.h"
#include "g_renderpass.h"
#include "g_swapchain.h"
#include "g_window.h"
#include "r_model.h"
#include "u_debug.h"

int main(int argc, char *argv[])
{
	LOG_INFO("Starting vulkan application");

	{
		GraphicsWindow window("Vulkan FPS", 800, 800);
		std::shared_ptr<GraphicsDevice> device = std::make_shared<GraphicsDevice>(window);

		GraphicsSwapchain swapchain(window, device);
		GraphicsDevmem devmem(device);
		
		std::vector<vk::UniqueCommandBuffer> command_buffers = device->graphics_queue.allocate_command_buffers(swapchain.get_image_count());

		vk::UniqueSemaphore acquire_semaphore = device->create_semaphore();
		vk::UniqueSemaphore release_semaphore = device->create_semaphore();

		GraphicsRenderpass renderpass(device);
		renderpass.add_attachment(vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(0),
			swapchain.get_swapchain_format().format, vk::SampleCountFlagBits::e1, 
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, 
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, 
			vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR)
		);
		std::vector<vk::AttachmentReference> attachments = { vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal) };
		renderpass.add_subpass(vk::SubpassDescription(
			vk::SubpassDescriptionFlags(0),
			vk::PipelineBindPoint::eGraphics,
			0, nullptr,
			(uint32_t) attachments.size(), attachments.data(), nullptr,
			nullptr,
			0, nullptr
		));
		renderpass.add_subpass_dependency(vk::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0, 
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlags(0), vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
		));
		renderpass.create_renderpass();

		std::vector<vk::UniqueFramebuffer> framebuffers;

		for (uint32_t i = 0; i < swapchain.get_image_count(); i++)
		{
			std::vector<vk::ImageView> views = { swapchain.get_image_view(i) };
			framebuffers.push_back(renderpass.create_framebuffer(device->device, views, swapchain.get_extent()));
		}

		Model sphere_model(device, devmem, renderpass, "models/sphere.obj");

		vk::Rect2D screen_area(vk::Offset2D(0, 0), swapchain.get_extent());
		std::vector<vk::ClearValue> clear_values = { vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f }) };
		
		// Record command buffers
		for (uint32_t i = 0; i < swapchain.get_image_count(); i++)
		{
			vk::CommandBuffer cmd = command_buffers[i].get();
			cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr));

			renderpass.begin_renderpass(cmd, framebuffers[i].get(), screen_area, clear_values);
			{
				cmd.setViewport(0, { vk::Viewport(0, 0, 800, 800) });
				cmd.setScissor(0, { vk::Rect2D({0, 0}, {800, 800}) });

				sphere_model.render(cmd);
			}
			renderpass.end_renderpass(cmd);

			cmd.end();
		}

		LOG_INFO("Setup vulkan application");

		// Main window loop
		while (!window.should_close())
		{
			window.poll_events();

			device->device.waitIdle();

			uint32_t image = swapchain.aquire_image(device->device, acquire_semaphore.get());

			device->graphics_queue.submit_commands(std::vector<vk::CommandBuffer>{command_buffers[image].get()}, acquire_semaphore, release_semaphore);

			swapchain.present_image(device->device, image, release_semaphore.get());
		}
	}

	LOG_INFO("Destroyed vulkan application");

	while (true);
}
