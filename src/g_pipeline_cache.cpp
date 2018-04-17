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

#include "g_pipeline_cache.h"
#include "u_defines.h"


GraphicsPipelineCache::GraphicsPipelineCache(std::shared_ptr<GraphicsDevice> device)
	: device(device)
{
	vk::PipelineCacheCreateInfo create_info(
		vk::PipelineCacheCreateFlags(0),
		0, nullptr
	);

	pipeline_cache = device->device.createPipelineCache(create_info);
}

GraphicsPipelineCache::~GraphicsPipelineCache()
{
	device->device.destroyPipelineCache(pipeline_cache);
}

std::unique_ptr<GraphicsPipeline> GraphicsPipelineCache::create_pipeline(vk::GraphicsPipelineCreateInfo create_info, GraphicsDynamicStateFlags dynamic_state, vk::PipelineLayout pipeline_layout, vk::DescriptorSet descriptor_set, vk::DescriptorSetLayout descriptor_set_layout)
{
	// TODO lookup from internal cache
	std::vector<vk::DynamicState> dynamic_states = GraphicsPipeline::get_dynamic_states(dynamic_state);
	vk::PipelineDynamicStateCreateInfo dynamic_state_info(
		vk::PipelineDynamicStateCreateFlags(0),
		(uint32_t)dynamic_states.size(), dynamic_states.data()
	);
	create_info.pDynamicState = &dynamic_state_info;

	return std::make_unique<GraphicsPipeline>(device, pipeline_cache, create_info, pipeline_layout, descriptor_set, descriptor_set_layout);
}

uint32_t GraphicsPipelineCache::hash_pipeline_info(vk::GraphicsPipelineCreateInfo create_info, GraphicsDynamicStateFlags dynamic_state)
{
	int prime = 31;
	int result = 1;
	result = prime * result ^ (VkFlags) dynamic_state;
	result = prime * result ^ (int) create_info.pInputAssemblyState->topology;
	// TODO shader hashing
	return result;
}

