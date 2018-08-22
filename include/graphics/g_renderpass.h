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
#include "g_pipeline.h"
#include "g_pipeline_builder.h"
#include "g_pipeline_cache.h"

class GraphicsRenderpass
{
public:
	explicit GraphicsRenderpass(std::shared_ptr<GraphicsDevice> &device, vk::DescriptorPool descriptor_pool);
	~GraphicsRenderpass();

	void add_attachment(vk::AttachmentDescription attachment);

	void add_subpass(vk::SubpassDescription subpass);
	void add_subpass_dependency(vk::SubpassDependency dependency);

	void create_renderpass();

	vk::Framebuffer create_framebuffer(vk::Device device, std::vector<vk::ImageView> image_views, vk::Extent2D extent) const;

	void begin_renderpass(vk::CommandBuffer command_buffer, vk::Framebuffer framebuffer, vk::Rect2D render_area, std::vector<vk::ClearValue> clear_values, vk::SubpassContents contents = vk::SubpassContents::eInline) const;
	void end_renderpass(vk::CommandBuffer command_buffer) const;

	std::unique_ptr<GraphicsPipeline> create_pipeline(std::unique_ptr<GraphicsPipelineCreateInfo> create_info);

	explicit operator vk::RenderPass() const { return renderpass; }

private:
	bool created;
	std::shared_ptr<GraphicsDevice> device;

	vk::Device parent;
	vk::RenderPass renderpass;
	vk::DescriptorPool descriptor_pool;

	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> dependencies;
	GraphicsPipelineCache pipeline_cache;
};
