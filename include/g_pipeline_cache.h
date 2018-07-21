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

#include <vulkan/vulkan.hpp>

#include "g_device.h"
#include "g_pipeline.h"
#include "g_pipeline_builder.h"

class GraphicsPipelineCache
{
public:
	GraphicsPipelineCache(std::shared_ptr<GraphicsDevice> device, vk::DescriptorPool descriptor_pool);
	~GraphicsPipelineCache();

    std::unique_ptr<GraphicsPipeline> create_pipeline(std::unique_ptr<GraphicsPipelineCreateInfo> create_info, vk::RenderPass renderpass);

private:
	std::shared_ptr<GraphicsDevice> device;

	GraphicsPipelineBuilder pipeline_builder;
	vk::PipelineCache pipeline_cache;

	std::size_t hash_pipeline_info(GraphicsPipelineCreateInfo create_info);
};
