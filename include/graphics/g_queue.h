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

class GraphicsFence;
class GraphicsDevice;

class GraphicsQueue
{
public:
	GraphicsQueue(GraphicsDevice *device, uint32_t queue_index);
	GraphicsQueue(GraphicsQueue & queue) = delete;
	~GraphicsQueue();

	vk::CommandBuffer allocate_command_buffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
	std::vector<vk::CommandBuffer> allocate_command_buffers(uint32_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;

	void submit_commands(std::vector<vk::CommandBuffer> command_buffers, vk::Semaphore& check_semaphore, vk::Semaphore& update_semaphore, std::unique_ptr<GraphicsFence>& fence) const;
    void free_command_buffers(vk::ArrayProxy<const vk::CommandBuffer> buffer) const;

    vk::Queue queue;

private:
	GraphicsDevice *device;

	vk::CommandPool command_pool;
};
