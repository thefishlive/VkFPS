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

#include "g_image_sampler.h"

GraphicsImageSampler::GraphicsImageSampler(
    std::shared_ptr<GraphicsDevice> &device,
    std::shared_ptr<GraphicsDevmemImage> &image,
    vk::SamplerAddressMode address_mode_u,
    vk::SamplerAddressMode address_mode_v,
    vk::SamplerAddressMode address_mode_w,
    vk::BorderColor border_color
)
    : device(device), image(image)
{
    image->transition_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::ImageViewCreateInfo view_create_info (
        vk::ImageViewCreateFlags(0),
        vk::Image(),
        vk::ImageViewType::e2D,
        image->get_format(),
        vk::ComponentMapping(
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        ),
        vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0, 1,
            0, 1
        )
    );

    image_view = image->create_image_view(view_create_info);

    vk::SamplerCreateInfo sampler_create_info(
        vk::SamplerCreateFlags(0),
        vk::Filter::eLinear,
        vk::Filter::eLinear,
        vk::SamplerMipmapMode::eLinear,
        address_mode_u,
        address_mode_v,
        address_mode_w,
        0,
        VK_FALSE, 0,
        VK_FALSE, vk::CompareOp::eNever,
        0, 0,
        border_color,
        VK_FALSE
    );

    sampler = device->device.createSampler(sampler_create_info);
}

GraphicsImageSampler::~GraphicsImageSampler()
{
    device->device.destroySampler(sampler);
    device->device.destroyImageView(image_view);
}

vk::DescriptorImageInfo GraphicsImageSampler::get_image_info() const
{
    return vk::DescriptorImageInfo(
        sampler,
        image_view,
        image->get_layout()
    );
}
