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

#include <iostream>

#define VMA_IMPLEMENTATION
#include "u_debug.h"
#include "u_defines.h"
#include "vk_mem_alloc.h"

static std::string format_memory_flags(vk::MemoryPropertyFlags flags)
{
	std::string output;

	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eDeviceLocal))
	{
		output += "DEVICE_LOCAL | ";
	}
	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eHostVisible))
	{
		output += "HOST_VISIBLE | ";
	}
	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eHostCached))
	{
		output += "HOST_CACHED | ";
	}
	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eHostCoherent))
	{
		output += "HOST_COHERENT | ";
	}
	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eLazilyAllocated))
	{
		output += "LAZILY_ALLOCATED | ";
	}
	if (BITMASK_HAS(flags, vk::MemoryPropertyFlagBits::eProtected))
	{
		output += "PROTECTED | ";
	}

	return output.substr(0, output.length() - 3);
}

GraphicsDevmemBuffer::GraphicsDevmemBuffer(
    std::shared_ptr<GraphicsDevice> device, VmaAllocator allocator, 
    VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo alloc_info,
    VkBuffer staging_buffer, VmaAllocation staging_allocation, VmaAllocationInfo staging_alloc_info)
	: device(device), buffer(buffer), allocator(allocator), 
        allocation(allocation), alloc_info(alloc_info), 
        staging_buffer(staging_buffer), staging_allocation(staging_allocation), staging_alloc_info(staging_alloc_info)
{
}

GraphicsDevmemBuffer::~GraphicsDevmemBuffer()
{
	vmaDestroyBuffer(allocator, (VkBuffer)buffer, allocation);
}

void GraphicsDevmemBuffer::map_memory(void **data) const
{
	DEBUG_ASSERT(this->is_visible());

    VkResult result;

    if (staging_buffer == vk::Buffer())
    {
        result = vmaMapMemory(allocator, allocation, data);
    }
    else
    {
        result = vmaMapMemory(allocator, staging_allocation, data);
    }

	if (result != VK_SUCCESS)
	{
		vk::throwResultException((vk::Result) result, "vmaMapMemory");
	}
}

void GraphicsDevmemBuffer::unmap_memory() const
{
	DEBUG_ASSERT(this->is_visible());

    if (staging_buffer == vk::Buffer())
    {
        vmaUnmapMemory(allocator, allocation);
    }
    else
    {
        vmaUnmapMemory(allocator, staging_allocation);
    }

}

void GraphicsDevmemBuffer::commit_memory() const
{
    if (staging_buffer == vk::Buffer())
    {
        return;
    }

    /*
     * Queue transfer from staging buffer to live buffer
     */
    LOG_INFO("Queue transfer from staging buffer to live buffer");

    auto batch = device->transfer_context->start_batch(GraphicsTransferHardwareDest::eTransfer);
    batch->blit_buffer_to_buffer(staging_buffer, buffer, { vk::BufferCopy(0, 0, this->alloc_info.size) });
    device->transfer_context->end_batch(std::move(batch), true);
}

vk::BufferView GraphicsDevmemBuffer::create_buffer_view(vk::BufferViewCreateInfo& create_info) const
{
	create_info.buffer = buffer;
	return device->device.createBufferView(create_info);
}

bool GraphicsDevmemBuffer::is_visible() const
{
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(allocator, alloc_info.memoryType, &memFlags);
	return BITMASK_HAS(memFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) || staging_buffer != vk::Buffer();
}

GraphicsDevmemImage::GraphicsDevmemImage(std::shared_ptr<GraphicsDevice> device, VmaAllocator allocator, VkImage image, VmaAllocation allocation, VmaAllocationInfo alloc_info)
	: image(image), device(device), allocator(allocator), allocation(allocation), alloc_info(alloc_info)
{
}

GraphicsDevmemImage::~GraphicsDevmemImage()
{
	vmaDestroyImage(allocator, (VkImage) image, allocation);
}

void GraphicsDevmemImage::map_memory(void** data) const
{
	DEBUG_ASSERT(this->is_visible());

	VkResult result = vmaMapMemory(allocator, allocation, data);
	if (result != VK_SUCCESS)
	{
		vk::throwResultException((vk::Result) result, "vmaMapMemory");
	}
}

void GraphicsDevmemImage::unmap_memory() const
{
	DEBUG_ASSERT(this->is_visible());

	vmaUnmapMemory(allocator, allocation);
}

bool GraphicsDevmemImage::is_visible() const
{
	VkMemoryPropertyFlags memFlags;
	vmaGetMemoryTypeProperties(allocator, alloc_info.memoryType, &memFlags);
	LOG_DEBUG("Memory Flags: %s", format_memory_flags(vk::MemoryPropertyFlags(memFlags)).c_str());
	return BITMASK_HAS(memFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

vk::ImageView GraphicsDevmemImage::create_image_view(vk::ImageViewCreateInfo& create_info) const
{
	create_info.image = image;
	return device->device.createImageView(create_info);
}

GraphicsDevmem::GraphicsDevmem(std::shared_ptr<GraphicsDevice>& device)
	: device(device)
{
	VmaAllocatorCreateInfo create_info {};
	create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
	create_info.physicalDevice = (VkPhysicalDevice) device->physical_deivce;
	create_info.device = (VkDevice) device->device;

	VkResult result = vmaCreateAllocator(&create_info, &allocator);
	if (result != VK_SUCCESS)
	{
		vk::throwResultException((vk::Result) result, "vmaCreateAllocator");
	}
}

GraphicsDevmem::~GraphicsDevmem()
{
	vmaDestroyAllocator(allocator);
}

std::unique_ptr<GraphicsDevmemBuffer> GraphicsDevmem::create_buffer(vk::BufferCreateInfo buffer_create_info, VmaAllocationCreateInfo alloc_create_info) const
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo alloc_info;
	VkBufferCreateInfo create_info = (VkBufferCreateInfo)buffer_create_info;
    create_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkResult result = vmaCreateBuffer(allocator, &create_info, &alloc_create_info, &buffer, &allocation, &alloc_info);
	if (result != VK_SUCCESS)
	{
		vk::throwResultException((vk::Result) result, "vmaCreateBuffer");
	}

    /* Create staging buffers for gpu only allocations */
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_allocatin = VK_NULL_HANDLE;
    VmaAllocationInfo staging_alloc_info;

    if (alloc_create_info.usage == VMA_MEMORY_USAGE_GPU_ONLY)
    {
        LOG_INFO("Allocating staging buffer");

        /* Update create info structs for staging buffer values */
        alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        create_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        result = vmaCreateBuffer(allocator, &create_info, &alloc_create_info, &staging_buffer, &staging_allocatin, &staging_alloc_info);
        if (result != VK_SUCCESS)
        {
            vk::throwResultException((vk::Result) result, "vmaCreateBuffer");
        }
    }
    
	return std::make_unique<GraphicsDevmemBuffer>(device, allocator, buffer, allocation, alloc_info, staging_buffer, staging_allocatin, staging_alloc_info);
}

std::unique_ptr<GraphicsDevmemImage> GraphicsDevmem::create_image(vk::ImageCreateInfo image_create_info, VmaAllocationCreateInfo alloc_create_info) const
{
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo alloc_info;
	VkImageCreateInfo create_info = (VkImageCreateInfo)image_create_info;

	VkResult result = vmaCreateImage(allocator, &create_info, &alloc_create_info, &image, &allocation, &alloc_info);
	if (result != VK_SUCCESS)
	{
		vk::throwResultException((vk::Result) result, "vmaCreateImage");
	}

	return std::make_unique<GraphicsDevmemImage>(device, allocator, image, allocation, alloc_info);
}
