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

#include "g_pipeline.h"

#include "u_debug.h"

GraphicsPipeline::GraphicsPipeline(std::shared_ptr<GraphicsDevice>& device, vk::GraphicsPipelineCreateInfo create_info, vk::PipelineLayout pipeline_layout, vk::DescriptorSet descriptor_set, vk::DescriptorSetLayout descriptor_set_layout)
	:  device(device), pipeline_layout(pipeline_layout), descriptor_set(descriptor_set), descriptor_set_layout(descriptor_set_layout)
{
	pipeline = device->device.createGraphicsPipeline(vk::PipelineCache(), create_info);
}

GraphicsPipeline::~GraphicsPipeline()
{
	device->device.destroyPipeline(pipeline);
	device->device.destroyPipelineLayout(pipeline_layout);
	device->device.destroyDescriptorSetLayout(descriptor_set_layout);
}

void GraphicsPipeline::bind_pipeline(vk::CommandBuffer cmd) const
{
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
}

void GraphicsPipeline::push_shader_data(vk::CommandBuffer cmd, int offset, vk::ShaderStageFlagBits stage, size_t size, void* data) const
{
	cmd.pushConstants(pipeline_layout, stage, offset, (uint32_t)size, data);
}

void GraphicsPipeline::update_descriptor_sets(std::vector<vk::WriteDescriptorSet> writes) const
{
	for (auto &write : writes)
	{
		write.dstSet = descriptor_set;
	}

	device->device.updateDescriptorSets(writes, {});
}
