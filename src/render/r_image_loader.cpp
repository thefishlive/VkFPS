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
#include "r_image_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "u_defines.h"
#include "u_debug.h"

std::shared_ptr<GraphicsDevmemImage> RenderImageLoader::load_image(std::string file) const
{
    int texture_width, texture_height, texture_channels;
    stbi_uc* pixels = stbi_load(file.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    VkDeviceSize image_size = texture_width * texture_height * 4;

    LOG_INFO("Loading image %s", file.c_str());

    if (!pixels) 
    {
        throw std::runtime_error("failed to load texture image!");
    }

    vk::BufferCreateInfo staging_buffer_create_info(
        vk::BufferCreateFlags(),
        image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    );

    VmaAllocationCreateInfo staging_buffer_alloc_info{};
    staging_buffer_alloc_info.flags = 0;
    staging_buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    staging_buffer_alloc_info.pUserData = STRING_TO_DATA("ImageStagingBuffer");

    std::unique_ptr<GraphicsDevmemBuffer> staging_buffer = devmem->create_buffer(staging_buffer_create_info, staging_buffer_alloc_info);

    void *data;
    staging_buffer->map_memory(&data);
    memcpy(data, pixels, image_size);
    staging_buffer->unmap_memory();

    stbi_image_free(pixels);

    vk::ImageCreateInfo image_create_info(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Unorm,
        vk::Extent3D(texture_width, texture_height, 1),
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined
    );

    VmaAllocationCreateInfo image_alloc_info{};
    image_alloc_info.flags = VMA_ALLOCATION_CREATE_EXT_DONT_CREATE_STAGING_BUFFER;
    image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    image_alloc_info.pUserData = STRING_TO_DATA(file.c_str());

    std::unique_ptr<GraphicsDevmemImage> image = devmem->create_image(image_create_info, image_alloc_info);
    image->transition_layout(vk::ImageLayout::eTransferDstOptimal);

    vk::BufferImageCopy region(
        0, 0,
        0, 
        vk::ImageSubresourceLayers(
            vk::ImageAspectFlagBits::eColor,
            0,
            0,
            1
        ),
        vk::Offset3D(
            0,
            0,
            0
        ),
        vk::Extent3D(
            texture_width,
            texture_height,
            1
        )
    );

    auto batch = device->transfer_context->start_batch(GraphicsTransferHardwareDest::eGraphics);
    batch->blit_buffer_to_image(staging_buffer->buffer, image->image, vk::ImageLayout::eTransferDstOptimal, region);
    device->transfer_context->end_batch(std::move(batch), false);

    device->device.waitIdle();

    return std::move(image);
}
