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
#include "vk_mem_alloc.h"

typedef enum VmaAllocationCreateFlagBitsExtended
{
    VMA_ALLOCATION_CREATE_EXT_DONT_CREATE_STAGING_BUFFER = 1 << 16,
} VmaAllocationCreateFlagBitsExtended;

class GraphicsDevmemBuffer
{
public:
	GraphicsDevmemBuffer(
        std::shared_ptr<GraphicsDevice> device, VmaAllocator allocator,
        VkBuffer buffer, VmaAllocation allocation, VmaAllocationInfo alloc_info,
        VkBuffer staging_buffer, VmaAllocation staging_allocation, VmaAllocationInfo staging_alloc_info
    );
	~GraphicsDevmemBuffer();

	bool is_visible() const;
	void map_memory(void **data) const;
	void unmap_memory() const;
    void commit_memory() const;

	vk::BufferView create_buffer_view(vk::BufferViewCreateInfo& create_info) const;

	vk::Buffer buffer;

private:
	std::shared_ptr<GraphicsDevice> device;
	VmaAllocator allocator;

	VmaAllocation allocation;
	VmaAllocationInfo alloc_info;

    vk::Buffer staging_buffer;
    VmaAllocation staging_allocation;
    VmaAllocationInfo staging_alloc_info;
};

class GraphicsDevmemImage
{
public:
	GraphicsDevmemImage(
        std::shared_ptr<GraphicsDevice> device,
        VmaAllocator allocator,
        VkImage image,
        VmaAllocation allocation,
        VmaAllocationInfo alloc_info,
        vk::ImageLayout layout,
        vk::Format format
    );
	~GraphicsDevmemImage();

	void map_memory(void **data) const;
	void unmap_memory() const;

	bool is_visible() const;
	vk::ImageView create_image_view(vk::ImageViewCreateInfo& create_info) const;

    vk::Format get_format() const { return format; };

    vk::ImageLayout get_layout() const;
    void transition_layout(vk::ImageLayout dest);

    vk::Image image;

private:
	std::shared_ptr<GraphicsDevice> device;
	VmaAllocator allocator;

	VmaAllocation allocation;
	VmaAllocationInfo alloc_info;

    vk::ImageLayout layout;
    vk::Format format;
};

class GraphicsDevmem
{
public:
	GraphicsDevmem(std::shared_ptr<GraphicsDevice>& device);
	~GraphicsDevmem();

	std::unique_ptr<GraphicsDevmemBuffer> create_buffer(vk::BufferCreateInfo buffer_create_info, VmaAllocationCreateInfo alloc_create_info) const;
	std::unique_ptr<GraphicsDevmemImage> create_image(vk::ImageCreateInfo create_info, VmaAllocationCreateInfo alloc_create_info) const;

private:
	std::shared_ptr<GraphicsDevice> device;
	VmaAllocator allocator;
};
