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

#include <memory>
#include <vector>

#include "g_queue.h"

class GraphicsDevice;

enum GraphicsTransferHardwareDest
{
    eGraphics,
    eTransfer,
};

struct BatchSubmissionInfo
{
    GraphicsTransferHardwareDest hw_dest;
    vk::Fence fence;
    std::vector<vk::Semaphore> update_semaphores;
    vk::CommandBuffer buffer;

    BatchSubmissionInfo(GraphicsTransferHardwareDest hw_dest, vk::Fence fence, std::vector<vk::Semaphore> update_semaphores, vk::CommandBuffer buffer)
        : hw_dest(hw_dest), fence(fence), update_semaphores(update_semaphores), buffer(buffer)
    {
    }
};

class GraphicsTransferBatch
{
public:
    explicit GraphicsTransferBatch(GraphicsTransferHardwareDest dest, vk::CommandBuffer cmd) : dest(dest), cmd(cmd) {}

    void pipeline_barrier(vk::PipelineStageFlags source_stage, vk::PipelineStageFlags dest_stage, vk::ArrayProxy<const vk::ImageMemoryBarrier> memory_barriers) const;
    void blit_buffer_to_buffer(vk::Buffer src, vk::Buffer dest, vk::ArrayProxy<const vk::BufferCopy> regions) const;
    void blit_image_to_image(vk::Image src, vk::ImageLayout src_layout, vk::Image dest, vk::ImageLayout dest_layout, vk::ArrayProxy<const vk::ImageCopy> regions) const;
    void blit_buffer_to_image(vk::Buffer src, vk::Image dest, vk::ImageLayout dest_layout, vk::ArrayProxy<const vk::BufferImageCopy> regions) const;
    void blit_image_to_buffer(vk::Image src, vk::ImageLayout src_layout, vk::Buffer dest, vk::ArrayProxy<const vk::BufferImageCopy> regions) const;

    explicit operator vk::CommandBuffer() const { return cmd; }
    GraphicsTransferHardwareDest dest;

private:
    vk::CommandBuffer cmd;
};

class GraphicsTransferContext
{
public:
    explicit GraphicsTransferContext(GraphicsDevice *device);

    std::unique_ptr<GraphicsTransferBatch> start_batch(GraphicsTransferHardwareDest dest) const;
    void end_batch(std::unique_ptr<GraphicsTransferBatch>, bool sync_frame);
    
    std::vector<vk::Semaphore> get_next_frame_sync();

private:
    GraphicsDevice *device;

    std::shared_ptr<GraphicsQueue> queue;
    std::vector<BatchSubmissionInfo> frame_syncs;
    std::vector<BatchSubmissionInfo> deffer_free_list;

    std::shared_ptr<GraphicsQueue> get_hw_queue(GraphicsTransferHardwareDest dest) const;
};
