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
#include "g_shader.h"
#include "r_shaderif.h"
#include "u_debug.h"

GraphicsRenderpass::GraphicsRenderpass(std::shared_ptr<GraphicsDevice>& device) : created(false), device(device)
{
	std::vector<vk::DescriptorPoolSize> pool_sizes {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 8)
	};

	vk::DescriptorPoolCreateInfo create_info(
		vk::DescriptorPoolCreateFlags(0),
		8,
		(uint32_t) pool_sizes.size(), pool_sizes.data()
	);

	descriptor_pool = device->device.createDescriptorPool(create_info);
}

GraphicsRenderpass::~GraphicsRenderpass()
{
	if (created)
	{
		parent.destroyRenderPass(renderpass);
		parent.destroyDescriptorPool(descriptor_pool);
	}
}

void GraphicsRenderpass::add_attachment(vk::AttachmentDescription attachment)
{
	DEBUG_ASSERT(!created);

	this->attachments.push_back(attachment);
}

void GraphicsRenderpass::add_subpass(vk::SubpassDescription subpass)
{
	DEBUG_ASSERT(!created);

	this->subpasses.push_back(subpass);
}

void GraphicsRenderpass::add_subpass_dependency(vk::SubpassDependency dependency)
{
	DEBUG_ASSERT(!created);

	this->dependencies.push_back(dependency);
}

void GraphicsRenderpass::create_renderpass()
{
	DEBUG_ASSERT(!created);

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
	DEBUG_ASSERT(created);

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
	DEBUG_ASSERT(created);

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
	DEBUG_ASSERT(created);

	command_buffer.endRenderPass();
}

std::unique_ptr<GraphicsPipeline> GraphicsRenderpass::create_pipeline(std::string vertex_shader_file, std::string frag_shader_file)
{
	LOG_INFO("Creating pipeline for shaders %s %s", vertex_shader_file.c_str(), frag_shader_file.c_str());

	std::vector<vk::DescriptorSetLayoutBinding> set_bindings = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment),
	};

	vk::DescriptorSetLayoutCreateInfo set_create_info(
		vk::DescriptorSetLayoutCreateFlags(0),
		(uint32_t)set_bindings.size(), set_bindings.data()
	);

	vk::DescriptorSetLayout descriptor_layout = device->device.createDescriptorSetLayout(set_create_info);

	vk::DescriptorSetAllocateInfo descriptor_set_alloc_info(
		descriptor_pool,
		1, &descriptor_layout
	);

	vk::DescriptorSet descriptor_set = device->device.allocateDescriptorSets(descriptor_set_alloc_info)[0];

	std::vector<vk::DescriptorSetLayout> descriptor_sets{
		descriptor_layout
	};

	std::vector<vk::PushConstantRange> push_constant_ranges{
		vk::PushConstantRange(
			vk::ShaderStageFlagBits::eVertex,
			0, sizeof(VertexShaderData)
		),
		vk::PushConstantRange(
			vk::ShaderStageFlagBits::eFragment,
			sizeof(VertexShaderData), sizeof(MaterialShaderData)
		),
	};
	
	vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
		vk::PipelineLayoutCreateFlags(0),
		(uint32_t)descriptor_sets.size(), descriptor_sets.data(),
		(uint32_t)push_constant_ranges.size(), push_constant_ranges.data()
	);

	vk::PipelineLayout pipeline_layout = device->device.createPipelineLayout(pipeline_layout_create_info);

	std::vector<vk::VertexInputBindingDescription> vertex_input_bindings = Vertex::get_vertex_input_bindings();
	std::vector<vk::VertexInputAttributeDescription> vertex_input_attributes = Vertex::get_vertex_input_attributes();

	vk::PipelineVertexInputStateCreateInfo vertex_input(
		vk::PipelineVertexInputStateCreateFlags(0),
		(uint32_t)vertex_input_bindings.size(), vertex_input_bindings.data(),
		(uint32_t)vertex_input_attributes.size(), vertex_input_attributes.data()
	);

	vk::PipelineInputAssemblyStateCreateInfo input_assembly(
		vk::PipelineInputAssemblyStateCreateFlags(0),
		vk::PrimitiveTopology::eTriangleList,
		false
	);

	GraphicsShader vert_shader(device, vertex_shader_file);

	vk::PipelineShaderStageCreateInfo vertex_stage(
		vk::PipelineShaderStageCreateFlags(0),
		vk::ShaderStageFlagBits::eVertex,
		(vk::ShaderModule) vert_shader,
		"main"
	);

	vk::PipelineTessellationStateCreateInfo tessellation(
		vk::PipelineTessellationStateCreateFlags(0),
		0
	);

	// Will be dynamically set
	std::vector<vk::Viewport> viewports{ vk::Viewport() };
	std::vector<vk::Rect2D> scissors{ vk::Rect2D() };

	vk::PipelineViewportStateCreateInfo viewport(
		vk::PipelineViewportStateCreateFlags(0),
		(uint32_t)viewports.size(), viewports.data(),
		(uint32_t)scissors.size(), scissors.data()
	);

	vk::PipelineRasterizationStateCreateInfo rasterization(
		vk::PipelineRasterizationStateCreateFlags(0),
		false,
		false,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eNone,
		vk::FrontFace::eCounterClockwise,
		0, 0, 0, 0,
		1
	);

	vk::PipelineMultisampleStateCreateInfo multisample(
		vk::PipelineMultisampleStateCreateFlags(0),
		vk::SampleCountFlagBits::e1
	);

	GraphicsShader frag_shader(device, frag_shader_file);

	vk::PipelineShaderStageCreateInfo frag_stage(
		vk::PipelineShaderStageCreateFlags(0),
		vk::ShaderStageFlagBits::eFragment,
		(vk::ShaderModule) frag_shader,
		"main"
	);

	vk::PipelineDepthStencilStateCreateInfo depth_stencil(
		vk::PipelineDepthStencilStateCreateFlags(0),
		false, false,
		vk::CompareOp::eGreaterOrEqual,
		false, false,
		vk::StencilOpState(), vk::StencilOpState(),
		0.0f, 1.0f
	);

	std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamic_state(
		vk::PipelineDynamicStateCreateFlags(0),
		(uint32_t)dynamic_states.size(), dynamic_states.data()
	);

	vk::PipelineColorBlendAttachmentState color_attachment_state(
		false,
		vk::BlendFactor::eOne,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		vk::BlendFactor::eOne,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	);

	std::vector<vk::PipelineColorBlendAttachmentState> color_attachments{ color_attachment_state };

	vk::PipelineColorBlendStateCreateInfo color_blend(
		vk::PipelineColorBlendStateCreateFlags(0),
		false, vk::LogicOp::eClear,
		(uint32_t)color_attachments.size(), color_attachments.data(),
		std::array<float, 4>{}
	);

	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = { vertex_stage, frag_stage };

	vk::GraphicsPipelineCreateInfo create_info;

	create_info.flags = vk::PipelineCreateFlags(0);

	create_info.renderPass = (vk::RenderPass) renderpass;
	create_info.layout = pipeline_layout;

	create_info.stageCount = (uint32_t)shader_stages.size();
	create_info.pStages = shader_stages.data();

	create_info.pVertexInputState = &vertex_input;
	create_info.pInputAssemblyState = &input_assembly;
	create_info.pTessellationState = &tessellation;
	create_info.pViewportState = &viewport;
	create_info.pRasterizationState = &rasterization;
	create_info.pMultisampleState = &multisample;
	create_info.pDepthStencilState = nullptr; // TODO
	create_info.pColorBlendState = &color_blend;
	create_info.pDynamicState = &dynamic_state;

	create_info.subpass = 0;
	create_info.basePipelineHandle = vk::Pipeline();

	return std::make_unique<GraphicsPipeline>(device, create_info, pipeline_layout, descriptor_set, descriptor_layout);
}
