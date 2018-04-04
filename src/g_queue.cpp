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

#include "g_queue.h"

#include "g_device.h"

GraphicsQueue::GraphicsQueue()
{
}

GraphicsQueue::~GraphicsQueue()
{
	if ((VkDevice) device != VK_NULL_HANDLE)
	{
		device.destroyCommandPool(command_pool);
	}
}

void GraphicsQueue::init_queue(vk::Device device, uint32_t queue_index)
{
	this->device = device;
	this->queue = device.getQueue(queue_index, 0);

	vk::CommandPoolCreateInfo create_info(
		vk::CommandPoolCreateFlags(0),
		queue_index
	);

	this->command_pool = device.createCommandPool(create_info);
}

std::vector<vk::UniqueCommandBuffer> GraphicsQueue::allocate_command_buffers(uint32_t count) const
{
	vk::CommandBufferAllocateInfo info(
		command_pool,
		vk::CommandBufferLevel::ePrimary,
		count
	);

	return device.allocateCommandBuffersUnique(info);
}

void GraphicsQueue::submit_commands(std::vector<vk::CommandBuffer> command_buffers, vk::UniqueSemaphore & check_semaphore, vk::UniqueSemaphore & update_semaphore) const
{
	std::array<vk::Semaphore, 1> check_semaphores = { check_semaphore.get() };
	std::array<vk::PipelineStageFlags, 1> pipeline_stage_flags = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	std::array<vk::Semaphore, 1> update_semaphores = { update_semaphore.get() };

	vk::SubmitInfo submit_info(
		(uint32_t) check_semaphores.size(), check_semaphores.data(), pipeline_stage_flags.data(),
		(uint32_t) command_buffers.size(), command_buffers.data(),
		(uint32_t) update_semaphores.size(), update_semaphores.data()
	);

	std::array<vk::SubmitInfo, 1> submits = { submit_info };

	queue.submit((uint32_t) submits.size(), submits.data(), vk::Fence());
}
