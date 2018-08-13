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

#include "g_transfer_context.h"

#include "g_device.h"

void GraphicsTransferBatch::pipeline_barrier(vk::PipelineStageFlags source_stage, vk::PipelineStageFlags dest_stage, vk::ArrayProxy<const vk::ImageMemoryBarrier> memory_barriers) const
{
    this->cmd.pipelineBarrier(source_stage, dest_stage, vk::DependencyFlags(), {}, {}, memory_barriers);
}

void GraphicsTransferBatch::blit_buffer_to_buffer(vk::Buffer src, vk::Buffer dest, vk::ArrayProxy<const vk::BufferCopy> regions) const
{
    this->cmd.copyBuffer(src, dest, regions);
}

void GraphicsTransferBatch::blit_image_to_image(vk::Image src, vk::ImageLayout src_layout, vk::Image dest, vk::ImageLayout dest_layout, vk::ArrayProxy<const vk::ImageCopy> regions) const
{
    this->cmd.copyImage(src, src_layout, dest, dest_layout, regions);
}

void GraphicsTransferBatch::blit_buffer_to_image(vk::Buffer src, vk::Image dest, vk::ImageLayout dest_layout, vk::ArrayProxy<const vk::BufferImageCopy> regions) const
{
    this->cmd.copyBufferToImage(src, dest, dest_layout, regions);
}

void GraphicsTransferBatch::blit_image_to_buffer(vk::Image src, vk::ImageLayout src_layout, vk::Buffer dest, vk::ArrayProxy<const vk::BufferImageCopy> regions) const
{
    this->cmd.copyImageToBuffer(src, src_layout, dest, regions);
}

GraphicsTransferContext::GraphicsTransferContext(GraphicsDevice *device)
    : device(device), queue(device->transfer_queue)
{
}

std::shared_ptr<GraphicsQueue> GraphicsTransferContext::get_hw_queue(GraphicsTransferHardwareDest dest) const
{
    switch (dest)
    {
    case eGraphics:
        return this->device->graphics_queue;
    case eTransfer:
        return this->device->transfer_queue;
    default:
        throw std::exception("Unknown hardware destination queue.");
    }
}

std::unique_ptr<GraphicsTransferBatch> GraphicsTransferContext::start_batch(GraphicsTransferHardwareDest dest) const
{
    std::shared_ptr<GraphicsQueue> queue = this->get_hw_queue(dest);

    vk::CommandBuffer buffer = queue->allocate_command_buffer(vk::CommandBufferLevel::ePrimary);
    buffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr });
    return std::make_unique<GraphicsTransferBatch>(dest, buffer);
}

void GraphicsTransferContext::end_batch(std::unique_ptr<GraphicsTransferBatch> batch, bool sync_frame)
{
    std::vector<vk::Semaphore> update_semaphores;
    vk::Fence fence = device->device.createFence({});

    if (sync_frame)
    {
        if (batch->dest != GraphicsTransferHardwareDest::eGraphics)
        {
            /*
            * Use semaphores for cross queue synchronisation
            */
            update_semaphores.push_back(device->create_semaphore());
        }
    }

    vk::CommandBuffer buffer = (vk::CommandBuffer) *batch;
    buffer.end();

    std::shared_ptr<GraphicsQueue> queue = this->get_hw_queue(batch->dest);
    std::vector<vk::CommandBuffer> commands{ buffer };

    vk::SubmitInfo submit_info (
        0, nullptr, nullptr,
        (uint32_t)commands.size(), commands.data(),
        (uint32_t)update_semaphores.size(), update_semaphores.data()
    );

    std::array<vk::SubmitInfo, 1> submits = { submit_info };

    frame_syncs.push_back(BatchSubmissionInfo(batch->dest, fence, update_semaphores, (vk::CommandBuffer) *batch));
    queue->queue.submit((uint32_t)submits.size(), submits.data(), fence);
}

std::vector<vk::Semaphore> GraphicsTransferContext::get_next_frame_sync()
{
    std::vector<BatchSubmissionInfo>::iterator e = deffer_free_list.begin();
    while (e != deffer_free_list.end())
    {
        auto result = device->device.getFenceStatus(e->fence);

        if (result == vk::Result::eSuccess)
        {
            // TODO destroy semaphores, but only after dependent commands have been executed
            //if (e->semaphore != vk::Semaphore())
            //{
            //    device->device.destroySemaphore(e->semaphore);
            //}

            device->device.destroyFence(e->fence);

            std::shared_ptr<GraphicsQueue> queue = this->get_hw_queue(e->hw_dest);
            queue->free_command_buffers(e->buffer);

            e = deffer_free_list.erase(e);
        }

        if (e == deffer_free_list.end())
        {
            break;
        }

        ++e;
    }

    std::vector<vk::Semaphore> semaphores;

    for (const auto & submission : frame_syncs)
    {
        for (const auto & semaphore : submission.update_semaphores)
        {
            semaphores.push_back(semaphore);
        }

        deffer_free_list.push_back(submission);
    }

    frame_syncs.clear();
    
    return semaphores;
}
