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

class GraphicsDevice;

class GraphicsQueue
{
public:
	explicit GraphicsQueue();
	GraphicsQueue(GraphicsQueue & queue) = delete;
	~GraphicsQueue();

	void init_queue(vk::Device device, uint32_t queue_index);

	std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(uint32_t count) const;

	void submit_commands(std::vector<vk::CommandBuffer> command_buffers, vk::UniqueSemaphore & check_semaphore, vk::UniqueSemaphore & update_semaphore) const;

	vk::Queue queue;

private:
	vk::Device device;

	vk::CommandPool command_pool;
};
