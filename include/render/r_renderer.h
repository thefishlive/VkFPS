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

#include "g_devmem.h"
#include "g_renderpass.h"
#include "g_swapchain.h"

struct RenderAttachment
{
	vk::Format format;
	vk::ImageView view;
	std::unique_ptr<GraphicsDevmemImage> image;
};

struct RenderAttachments
{
	RenderAttachment position;
	RenderAttachment normal;
	RenderAttachment color;
	RenderAttachment depth;
};

class Renderer
{
public:
	Renderer(std::shared_ptr<GraphicsDevice> device, std::shared_ptr<GraphicsWindow> window, std::shared_ptr<GraphicsDevmem> devmem, std::shared_ptr<GraphicsSwapchain> swapchain);
	~Renderer();

	vk::Framebuffer get_framebuffer(uint32_t index) const { return framebuffers[index]; }

	void begin_renderpass(vk::CommandBuffer cmd, uint32_t image_index);
	void next_subpass(vk::CommandBuffer cmd);
	void end_renderpass(vk::CommandBuffer cmd) const;

	std::vector<vk::CommandBuffer> alloc_render_command_buffers() const;
	void start_secondary_command_buffer(vk::CommandBuffer cmd, uint32_t index, uint32_t subpass) const;
	void end_secondary_command_buffer(vk::CommandBuffer cmd) const;

	void render_final_image(const vk::CommandBuffer& cmd, uint32_t index);

	std::shared_ptr<GraphicsRenderpass> get_renderpass() const;

private:
	std::shared_ptr<GraphicsDevice> device;
	std::shared_ptr<GraphicsWindow> window;
	std::shared_ptr<GraphicsDevmem> devmem;
	std::shared_ptr<GraphicsSwapchain> swapchain;

	std::shared_ptr<GraphicsRenderpass> renderpass;
	std::vector<vk::Framebuffer> framebuffers;
	RenderAttachments attachments;
	std::vector<vk::CommandBuffer> command_buffers;

    std::unique_ptr<GraphicsPipeline> deferred_pipeline;
    vk::DescriptorPool deferred_descriptor_pool;
    std::unique_ptr<GraphicsDevmemBuffer> screen_vertex_buffer;
    vk::Sampler deferred_sampler;

    RenderAttachment create_attachment(vk::Format format, vk::ImageUsageFlags usage, std::string attachment_name) const;
    void update_buffer_descriptor_sets() const;
	void create_lighting_pass_resources();
    std::unique_ptr<GraphicsPipeline> create_deffered_pipeline();

	static vk::Format pick_depth_buffer_format(std::shared_ptr<GraphicsDevice> device);
};
