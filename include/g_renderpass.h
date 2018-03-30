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

class GraphicsRenderpass
{
public:
	GraphicsRenderpass();
	~GraphicsRenderpass();

	void add_attachment(vk::AttachmentDescription attachment);

	void add_subpass(vk::SubpassDescription subpass);
	void add_subpass_dependency(vk::SubpassDependency dependency);

	void create_renderpass(vk::Device);

	vk::UniqueFramebuffer create_framebuffer(vk::Device device, std::vector<vk::ImageView> image_views, vk::Extent2D extent);

	void begin_renderpass(vk::CommandBuffer command_buffer, vk::Framebuffer framebuffer, vk::Rect2D render_area, std::vector<vk::ClearValue> clear_values);
	void end_renderpass(vk::CommandBuffer command_buffer) const;

private:
	bool created;
	vk::UniqueRenderPass renderpass;

	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> dependencies;
};
