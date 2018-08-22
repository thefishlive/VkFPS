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


GraphicsPipelineCache::GraphicsPipelineCache(std::shared_ptr<GraphicsDevice> device, vk::DescriptorPool descriptor_pool)
	: device(device), pipeline_builder(device, descriptor_pool)
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

std::unique_ptr<GraphicsPipeline> GraphicsPipelineCache::create_pipeline(std::unique_ptr<GraphicsPipelineCreateInfo> create_info, vk::RenderPass renderpass)
{
	return pipeline_builder.create_pipeline(pipeline_cache, std::move(create_info), renderpass);
}

std::size_t GraphicsPipelineCache::hash_pipeline_info(GraphicsPipelineCreateInfo create_info)
{
	return std::hash<GraphicsPipelineCreateInfo>{}(create_info);
}

