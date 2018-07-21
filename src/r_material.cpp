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
#include "r_camera.h"
#include "r_scene.h"

std::unique_ptr<GraphicsPipelineCreateInfo> Material::get_pipeline_create_info(std::shared_ptr<GraphicsDevice>& device)
{
    std::unique_ptr<GraphicsPipelineCreateInfo> create_info = std::make_unique<GraphicsPipelineCreateInfo>(device, "shaders/model.vert", "shaders/model.frag");
    create_info->dynamic_states = GraphicsDynamicStateBits::ViewportBit | GraphicsDynamicStateBits::ScissorBit;

    std::vector<vk::DescriptorPoolSize> pool_sizes{
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2)
    };

    vk::DescriptorPoolCreateInfo descriptor_pool_create_info(
        vk::DescriptorPoolCreateFlags(0),
        1,
        (uint32_t)pool_sizes.size(), pool_sizes.data()
    );

    this->descriptor_pool = device->device.createDescriptorPool(descriptor_pool_create_info);

    std::vector<vk::DescriptorSetLayoutBinding> set_bindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment),
    };

    vk::DescriptorSetLayoutCreateInfo set_create_info(
        vk::DescriptorSetLayoutCreateFlags(0),
        (uint32_t)set_bindings.size(), set_bindings.data()
    );

    create_info->descriptor_set_layout = device->device.createDescriptorSetLayout(set_create_info);

    vk::DescriptorSetAllocateInfo descriptor_set_alloc_info(
        descriptor_pool,
        1, &create_info->descriptor_set_layout
    );

    create_info->descriptor_set = device->device.allocateDescriptorSets(descriptor_set_alloc_info)[0];

    std::vector<vk::DescriptorSetLayout> descriptor_sets{
        create_info->descriptor_set_layout
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

    create_info->pipeline_layout = device->device.createPipelineLayout(pipeline_layout_create_info);
    
    create_info->color_attachments = {
        // Final Presented Image
        vk::PipelineColorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ),
        // Color G-Buffer
        vk::PipelineColorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
        ),
        // Position G-Buffer
        vk::PipelineColorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
        ),
        // Normal G-Buffer
        vk::PipelineColorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ),
    };

    create_info->subpass = 0;

    create_info->depth_enable = true;

    return create_info;
}

Material::Material(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<Renderer>& renderer, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float alpha)
	: device(device), 
      renderer(renderer), 
      pipeline(std::move(renderer->get_renderpass()->create_pipeline(std::move(this->get_pipeline_create_info(device))))), 
      shader_data(ambient, diffuse, specular, alpha)
{
	vk::DescriptorBufferInfo camera_buffer = Camera::get()->get_buffer_info();
	vk::DescriptorBufferInfo light_buffer = Scene::get()->get_light_data_info();

	std::vector<vk::WriteDescriptorSet> writes{
		vk::WriteDescriptorSet(
			vk::DescriptorSet(),
			0,
			0,
			1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&camera_buffer,
			nullptr
		),
		vk::WriteDescriptorSet(
			vk::DescriptorSet(),
			1,
			0,
			1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&light_buffer,
			nullptr
		)
	};

	this->pipeline->update_descriptor_sets(writes);
}

Material::~Material()
{
    this->device->device.destroyDescriptorPool(descriptor_pool);
}

void Material::bind_material(vk::CommandBuffer cmd) const
{
	cmd.setViewport(0, { vk::Viewport(0, 0, 800, 800) });
	cmd.setScissor(0, { vk::Rect2D({ 0, 0 },{ 800, 800 }) });

	pipeline->bind_pipeline(cmd);

	this->pipeline->push_shader_data(cmd, sizeof(VertexShaderData), vk::ShaderStageFlagBits::eFragment, sizeof(MaterialShaderData), (void *)&shader_data);
}

void Material::push_shader_data(vk::CommandBuffer cmd, int offset, vk::ShaderStageFlagBits stage, size_t size, void* data) const
{
	this->pipeline->push_shader_data(cmd, offset, stage, size, data);
}
