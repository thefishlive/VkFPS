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
#include "g_fence.h"

GraphicsQueue::GraphicsQueue(GraphicsDevice *device, uint32_t queue_index)
    : device(device)
{
    queue = device->device.getQueue(queue_index, 0);
    vk::CommandPoolCreateInfo create_info(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queue_index
    );

    this->command_pool = device->device.createCommandPool(create_info);
}

GraphicsQueue::~GraphicsQueue()
{
    device->device.destroyCommandPool(command_pool);
}

vk::CommandBuffer GraphicsQueue::allocate_command_buffer(vk::CommandBufferLevel level) const
{
	return this->allocate_command_buffers(1, level)[0];
}

std::vector<vk::CommandBuffer> GraphicsQueue::allocate_command_buffers(uint32_t count, vk::CommandBufferLevel level) const
{
	vk::CommandBufferAllocateInfo info(
		command_pool,
		level,
		count
	);

	return device->device.allocateCommandBuffers(info);
}

void GraphicsQueue::free_command_buffers(vk::ArrayProxy<const vk::CommandBuffer> buffer) const
{
    device->device.freeCommandBuffers(command_pool, buffer);
}

void GraphicsQueue::submit_commands(std::vector<vk::CommandBuffer> command_buffers, vk::Semaphore& check_semaphore, vk::Semaphore& update_semaphore, std::unique_ptr<GraphicsFence>& fence) const
{
    std::vector<vk::Semaphore> check_semaphores;
	std::vector<vk::PipelineStageFlags> pipeline_stage_flags = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    /* Add check semaphore to batch */
    check_semaphores.push_back(check_semaphore);
    pipeline_stage_flags.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    /* Add transfer semaphore if required */
    for (const auto & transfer_semaphore : device->transfer_context->get_next_frame_sync())
    {
        check_semaphores.push_back(transfer_semaphore);
        pipeline_stage_flags.push_back(vk::PipelineStageFlagBits::eTopOfPipe);
    }
    
	std::array<vk::Semaphore, 1> update_semaphores = { update_semaphore };

	vk::SubmitInfo submit_info(
		(uint32_t) check_semaphores.size(), check_semaphores.data(), pipeline_stage_flags.data(),
		(uint32_t) command_buffers.size(), command_buffers.data(),
		(uint32_t) update_semaphores.size(), update_semaphores.data()
	);

	std::array<vk::SubmitInfo, 1> submits = { submit_info };
    fence->set_submitted();

	queue.submit((uint32_t) submits.size(), submits.data(), *fence.get());
}
