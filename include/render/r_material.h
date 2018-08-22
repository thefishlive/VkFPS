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

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "g_device.h"
#include "g_pipeline.h"
#include "g_renderpass.h"
#include "r_renderer.h"
#include "g_shaderif.h"
#include "g_image_sampler.h"

class Material
{
public:
    Material(
        std::shared_ptr<GraphicsDevice>& device,
        std::shared_ptr<Renderer> &renderer,
        glm::vec4 ambient,
        glm::vec4 diffuse,
        glm::vec4 specular,
        float alpha,
        std::shared_ptr<GraphicsDevmemImage> &ambient_texture,
        std::shared_ptr<GraphicsDevmemImage> &diffuse_texture,
        std::shared_ptr<GraphicsDevmemImage> &specular_image);

	~Material();

	void bind_material(vk::CommandBuffer buffer) const;
	void push_shader_data(vk::CommandBuffer cmd, int binding, vk::ShaderStageFlagBits stage, size_t size, void* data) const;

private:
	std::shared_ptr<GraphicsDevice> device;
	std::shared_ptr<Renderer> renderer;

	std::unique_ptr<GraphicsPipeline> pipeline;

    std::shared_ptr<GraphicsDevmemImage> ambient_texture;
    std::unique_ptr<GraphicsImageSampler> ambient_sampler;
    std::shared_ptr<GraphicsDevmemImage> diffuse_texture;
    std::unique_ptr<GraphicsImageSampler> diffuse_sampler;
    std::shared_ptr<GraphicsDevmemImage> specular_texture;
    std::unique_ptr<GraphicsImageSampler> specular_sampler;

	MaterialShaderData shader_data;
    vk::DescriptorPool descriptor_pool;

    std::unique_ptr<GraphicsPipelineCreateInfo> get_pipeline_create_info(std::shared_ptr<GraphicsDevice>& device);
    void write_descriptor_update();
};
