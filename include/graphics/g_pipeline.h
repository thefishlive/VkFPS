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

enum class GraphicsDynamicStateBits
{
	ViewportBit		= 1 << (int) vk::DynamicState::eViewport,
	ScissorBit		= 1 << (int) vk::DynamicState::eScissor,
	LineWidthBit		= 1 << (int) vk::DynamicState::eLineWidth,
	DepthBiasBit		= 1 << (int) vk::DynamicState::eDepthBias,
	BlendConstantsBit	= 1 << (int) vk::DynamicState::eBlendConstants,
	DepthBoundsBit		= 1 << (int) vk::DynamicState::eDepthBounds,
	StencilCompareMaskBit	= 1 << (int) vk::DynamicState::eStencilCompareMask,
	StencilWriteMaskBit	= 1 << (int) vk::DynamicState::eStencilWriteMask,
	StencilReferenceBit	= 1 << (int) vk::DynamicState::eStencilReference,
};

using GraphicsDynamicStateFlags = vk::Flags<GraphicsDynamicStateBits, VkFlags>;

VULKAN_HPP_INLINE GraphicsDynamicStateFlags operator|(GraphicsDynamicStateBits bit0, GraphicsDynamicStateBits bit1)
{
	return GraphicsDynamicStateFlags(bit0) | bit1;
}

VULKAN_HPP_INLINE GraphicsDynamicStateFlags operator~(GraphicsDynamicStateBits bits)
{
	return ~(GraphicsDynamicStateBits(bits));
}

class GraphicsPipeline
{
public:
	GraphicsPipeline(std::shared_ptr<GraphicsDevice>& device, vk::PipelineCache cache, vk::GraphicsPipelineCreateInfo create_info, vk::PipelineLayout pipeline_layout, vk::DescriptorSet descriptor_set, vk::DescriptorSetLayout descriptor_set_layout);
	~GraphicsPipeline();

	void bind_pipeline(vk::CommandBuffer cmd) const;
	void push_shader_data(vk::CommandBuffer cmd, int offset, vk::ShaderStageFlagBits stage, size_t size, void* data) const;

	void update_descriptor_sets(std::vector<vk::WriteDescriptorSet> writes) const;

	static std::vector<vk::DynamicState> GraphicsPipeline::get_dynamic_states(GraphicsDynamicStateFlags mask);
private:
	std::shared_ptr<GraphicsDevice>& device;

	vk::Pipeline pipeline;
	vk::PipelineLayout pipeline_layout;
	vk::DescriptorSet descriptor_set;
	vk::DescriptorSetLayout descriptor_set_layout;

};
