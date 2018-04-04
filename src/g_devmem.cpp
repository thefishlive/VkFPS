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

#include "g_devmem.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include <iostream>

GraphicsDevmemBuffer::GraphicsDevmemBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation)
	: buffer(buffer), allocator(allocator), allocation(allocation)
{
}

GraphicsDevmemBuffer::~GraphicsDevmemBuffer()
{
#ifndef NDEBUG
	uint32_t *alloc_ptr = (uint32_t *) &allocation;
	if (*alloc_ptr == 0xCAFEBABE)
	{
		std::cerr << "Double free on a devmem buffer" << std::endl;
		return;
	}
#endif
	vmaDestroyBuffer(allocator, (VkBuffer)buffer, allocation);

#ifndef NDEBUG
	allocation = {};
	memset(&allocation, 0xCAFEBABE, sizeof(uint32_t));
	buffer = vk::Buffer();
#endif
}

void GraphicsDevmemBuffer::map_buffer(void **data) const
{
	VkResult result = vmaMapMemory(allocator, allocation, data);
	if (result != VK_SUCCESS)
	{
		throw std::exception("Error unmapping buffer");
	}
}

void GraphicsDevmemBuffer::unmap_buffer() const
{
	vmaUnmapMemory(allocator, allocation);
}

GraphicsDevmem::GraphicsDevmem(std::shared_ptr<GraphicsDevice>& device)
{
	VmaAllocatorCreateInfo create_info {};
	create_info.physicalDevice = (VkPhysicalDevice) device->physical_deivce;
	create_info.device = (VkDevice) device->device;

	VkResult result = vmaCreateAllocator(&create_info, &allocator);
	if (result != VK_SUCCESS)
	{
		throw std::exception("Error creating allocator");
	}
}

GraphicsDevmem::~GraphicsDevmem()
{
	vmaDestroyAllocator(allocator);
}

GraphicsDevmemBuffer* GraphicsDevmem::create_buffer(vk::BufferCreateInfo buffer_create_info, VmaAllocationCreateInfo alloc_create_info) const
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation;
	VkBufferCreateInfo create_info = (VkBufferCreateInfo)buffer_create_info;
	
	VkResult result = vmaCreateBuffer(allocator, &create_info, &alloc_create_info, &buffer, &allocation, nullptr);
	if (result != VK_SUCCESS)
	{
		throw std::exception("Error creating allocation");
	}

	return new GraphicsDevmemBuffer(allocator, buffer, allocation);
}

