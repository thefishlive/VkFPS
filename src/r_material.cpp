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

Material::Material(std::shared_ptr<GraphicsDevice>& device, GraphicsRenderpass& renderpass, glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float alpha)
	: device(device), pipeline(std::move(renderpass.create_pipeline("shaders/standard.vert", "shaders/standard.frag"))), shader_data(ambient, diffuse, specular, alpha)
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
