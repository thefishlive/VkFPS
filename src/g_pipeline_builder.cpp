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
#include "g_pipeline_builder.h"

#include "g_shader.h"
#include "r_shaderif.h"
#include "u_debug.h"

GraphicsPipelineBuilder::GraphicsPipelineBuilder(std::shared_ptr<GraphicsDevice>& device, vk::DescriptorPool descriptor_pool)
	: device(device), descriptor_pool(descriptor_pool)
{
}

GraphicsPipelineBuilder::~GraphicsPipelineBuilder()
{
}

std::unique_ptr<GraphicsPipeline> GraphicsPipelineBuilder::create_pipeline(vk::PipelineCache cache, std::unique_ptr<GraphicsPipelineCreateInfo> create_info, vk::RenderPass renderpass)
{
    LOG_INFO("Creating new pipeline (vertex_shader=%s,fragment_shader=%s)", create_info->vertex_shader.get_shader_name().c_str(), create_info->fragment_shader.get_shader_name().c_str());

    std::vector<vk::VertexInputBindingDescription> vertex_input_bindings = Vertex::get_vertex_input_bindings();
    std::vector<vk::VertexInputAttributeDescription> vertex_input_attributes = Vertex::get_vertex_input_attributes();

    vk::PipelineVertexInputStateCreateInfo vertex_input(
        vk::PipelineVertexInputStateCreateFlags(0),
        (uint32_t)vertex_input_bindings.size(), vertex_input_bindings.data(),
        (uint32_t)vertex_input_attributes.size(), vertex_input_attributes.data()
    );

    vk::PipelineInputAssemblyStateCreateInfo input_assembly(
        vk::PipelineInputAssemblyStateCreateFlags(0),
        create_info->primitive_topology,
        false
    );

    vk::PipelineShaderStageCreateInfo vertex_stage(
        vk::PipelineShaderStageCreateFlags(0),
        vk::ShaderStageFlagBits::eVertex,
        (vk::ShaderModule) create_info->vertex_shader,
        "main"
    );

    vk::PipelineTessellationStateCreateInfo tessellation(
        vk::PipelineTessellationStateCreateFlags(0),
        0
    );

    // Will be dynamically set
    std::vector<vk::Viewport> viewports = create_info->viewports;
    std::vector<vk::Rect2D> scissors = create_info->scissors;

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
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise,
        0, 0, 0, 0,
        1
    );

    vk::PipelineMultisampleStateCreateInfo multisample(
        vk::PipelineMultisampleStateCreateFlags(0),
        vk::SampleCountFlagBits::e1
    );

    vk::PipelineShaderStageCreateInfo frag_stage(
        vk::PipelineShaderStageCreateFlags(0),
        vk::ShaderStageFlagBits::eFragment,
        (vk::ShaderModule) create_info->fragment_shader,
        "main"
    );

    vk::PipelineDepthStencilStateCreateInfo depth_stencil(
        vk::PipelineDepthStencilStateCreateFlags(0),
        true, true,
        vk::CompareOp::eLess,
        false, false,
        vk::StencilOpState(), vk::StencilOpState(),
        0.0f, 1.0f
    );
    
    vk::PipelineColorBlendStateCreateInfo color_blend(
        vk::PipelineColorBlendStateCreateFlags(0),
        false, vk::LogicOp::eClear,
        (uint32_t)create_info->color_attachments.size(), create_info->color_attachments.data(),
        std::array<float, 4>{}
    );

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages = { vertex_stage, frag_stage };

    std::vector<vk::DynamicState> dynamic_states = GraphicsPipeline::get_dynamic_states(create_info->dynamic_states);
    vk::PipelineDynamicStateCreateInfo dynamic_state(
        vk::PipelineDynamicStateCreateFlags(0),
        (uint32_t)dynamic_states.size(), dynamic_states.data()
    );

    vk::GraphicsPipelineCreateInfo vk_create_info;

    vk_create_info.flags = vk::PipelineCreateFlags(0);

    vk_create_info.stageCount = (uint32_t)shader_stages.size();
    vk_create_info.pStages = shader_stages.data();

    vk_create_info.pVertexInputState = &vertex_input;
    vk_create_info.pInputAssemblyState = &input_assembly;
    vk_create_info.pTessellationState = &tessellation;
    vk_create_info.pViewportState = &viewport;
    vk_create_info.pRasterizationState = &rasterization;
    vk_create_info.pMultisampleState = &multisample;
    vk_create_info.pColorBlendState = &color_blend;
    vk_create_info.pDynamicState = dynamic_states.size() > 0 ? &dynamic_state : nullptr;

    if (create_info->depth_enable)
    {
        vk_create_info.pDepthStencilState = &depth_stencil;
    }
    else
    {
        vk_create_info.pDepthStencilState = nullptr;
    }

    vk_create_info.layout = create_info->pipeline_layout;

    vk_create_info.renderPass = create_info->renderpass;
    vk_create_info.subpass = create_info->subpass;
    vk_create_info.basePipelineHandle = vk::Pipeline();

	return std::make_unique<GraphicsPipeline>(device, cache, vk_create_info, create_info->pipeline_layout, create_info->descriptor_set, create_info->descriptor_set_layout);
}
