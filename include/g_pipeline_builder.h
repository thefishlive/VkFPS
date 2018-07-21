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

#include <string>

#include <vulkan/vulkan.hpp>

#include "g_pipeline.h"
#include "g_shader.h"

struct GraphicsPipelineCreateInfo
{
	GraphicsPipelineCreateInfo(std::shared_ptr<GraphicsDevice>& device, std::string vertex_shader, std::string fragment_shader)
		: vertex_shader(device, vertex_shader),
		  fragment_shader(device, fragment_shader),
          primitive_topology(vk::PrimitiveTopology::eTriangleList),
          dynamic_states(GraphicsDynamicStateFlags(0)),
          viewports({vk::Viewport()}),
          scissors({vk::Rect2D()})
	{
	}

	GraphicsShader vertex_shader;
	GraphicsShader fragment_shader;

	vk::PrimitiveTopology primitive_topology;
	GraphicsDynamicStateFlags dynamic_states;

	std::vector<vk::Viewport> viewports;
	std::vector<vk::Rect2D> scissors;

    vk::DescriptorSet descriptor_set;
    vk::DescriptorSetLayout descriptor_set_layout;
    vk::PipelineLayout pipeline_layout;

    bool depth_enable;

    vk::RenderPass renderpass;
    uint32_t subpass;
    std::vector<vk::PipelineColorBlendAttachmentState> color_attachments;
};

namespace std
{
	template<> struct hash<GraphicsPipelineCreateInfo>
	{
		typedef GraphicsPipelineCreateInfo argument_type;
		typedef std::size_t result_type;

		result_type operator()(argument_type const& info) const noexcept
		{
			// TODO
			result_type const h1((VkFlags) info.dynamic_states);
			result_type const h2((VkFlags) info.primitive_topology);
            result_type const h3(info.subpass);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}

class GraphicsPipelineBuilder
{
public:
	GraphicsPipelineBuilder(std::shared_ptr<GraphicsDevice>& device, vk::DescriptorPool descriptor_poo);
	~GraphicsPipelineBuilder();
    std::unique_ptr<GraphicsPipeline> create_pipeline(vk::PipelineCache cache, std::unique_ptr<GraphicsPipelineCreateInfo> create_info, vk::RenderPass renderpass);

private:
	std::shared_ptr<GraphicsDevice> device;

    vk::DescriptorPool descriptor_pool;
};
