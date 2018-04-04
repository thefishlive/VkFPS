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

#include "g_renderpass.h"
#include "g_device.h"

GraphicsRenderpass::GraphicsRenderpass(std::shared_ptr<GraphicsDevice>& device) : created(false), device(device)
{
}

GraphicsRenderpass::~GraphicsRenderpass()
{
	if (created)
	{
		parent.destroyRenderPass(renderpass);
	}
}

void GraphicsRenderpass::add_attachment(vk::AttachmentDescription attachment)
{
	assert(!created);

	this->attachments.push_back(attachment);
}

void GraphicsRenderpass::add_subpass(vk::SubpassDescription subpass)
{
	assert(!created);

	this->subpasses.push_back(subpass);
}

void GraphicsRenderpass::add_subpass_dependency(vk::SubpassDependency dependency)
{
	assert(!created);

	this->dependencies.push_back(dependency);
}

void GraphicsRenderpass::create_renderpass()
{
	assert(!created);

	vk::RenderPassCreateInfo create_info(
		vk::RenderPassCreateFlags(0),
		(uint32_t)attachments.size(), attachments.data(),
		(uint32_t)subpasses.size(), subpasses.data(),
		(uint32_t)dependencies.size(), dependencies.data()
	);

	renderpass = device->device.createRenderPass(create_info);
	created = true;
}

vk::UniqueFramebuffer GraphicsRenderpass::create_framebuffer(vk::Device device, std::vector<vk::ImageView> image_views, vk::Extent2D extent) const
{
	assert(created);

	vk::FramebufferCreateInfo create_info(
		vk::FramebufferCreateFlags(0),
		renderpass,
		(uint32_t)image_views.size(), image_views.data(),
		extent.width, extent.height, 1
	);

	return device.createFramebufferUnique(create_info);
}

void GraphicsRenderpass::begin_renderpass(vk::CommandBuffer command_buffer, vk::Framebuffer framebuffer, vk::Rect2D render_area, std::vector<vk::ClearValue> clear_values) const
{
	assert(created);

	vk::RenderPassBeginInfo info(
		renderpass,
		framebuffer,
		render_area,
		(uint32_t)clear_values.size(), clear_values.data()
	);

	command_buffer.beginRenderPass(info, vk::SubpassContents::eInline);
}

void GraphicsRenderpass::end_renderpass(vk::CommandBuffer command_buffer) const
{
	assert(created);

	command_buffer.endRenderPass();
}
