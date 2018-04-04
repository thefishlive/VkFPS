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

#include "r_material.h"
#include "g_shader.h"

Material::Material(std::shared_ptr<GraphicsDevice>& device, GraphicsRenderpass& renderpass, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float alpha)
	: device(device), pipeline(device)
{
	vk::PushConstantRange push_constant_range(
		vk::ShaderStageFlagBits::eFragment,
		0, sizeof(MaterialShaderData)
	);

	std::vector<vk::PushConstantRange> push_constant_ranges{ push_constant_range };

	vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
		vk::PipelineLayoutCreateFlags(0),
		0, nullptr,
		(uint32_t)push_constant_ranges.size(), push_constant_ranges.data()
	);

	pipeline_layout = device->device.createPipelineLayout(pipeline_layout_create_info);

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

	GraphicsShader vert_shader(device, "shaders/standard.vert");

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

	GraphicsShader frag_shader(device, "shaders/standard.frag");

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

	std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamic_state(
		vk::PipelineDynamicStateCreateFlags(0),
		(uint32_t)dynamic_states.size(), dynamic_states.data()
	);

	std::vector<vk::PipelineColorBlendAttachmentState> color_attachments { color_attachment_state };

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

	pipeline.create_pipeline(create_info);
}

Material::~Material()
{
	device->device.destroyPipelineLayout(pipeline_layout);
}

void Material::bind_material(vk::CommandBuffer cmd) const
{
	pipeline.bind_pipeline(cmd);

	cmd.setViewport(0, { vk::Viewport(0, 0, 800, 800) });
	cmd.setScissor(0, { vk::Rect2D({ 0, 0 },{ 800, 800 }) });
}
