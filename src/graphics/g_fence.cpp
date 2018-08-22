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
#include "g_fence.h"

#include "g_device.h"

GraphicsFence::GraphicsFence(std::shared_ptr<GraphicsDevice> device)
    : device(device), status(GraphicsFenceStatus::Complete)
{
    vk::FenceCreateInfo create_info(
        vk::FenceCreateFlagBits::eSignaled
    );
    fence = device->device.createFence(create_info);
}

GraphicsFence::~GraphicsFence()
{
    device->device.destroyFence(fence);
}

GraphicsFenceStatus GraphicsFence::get_status() const
{
    if (status == GraphicsFenceStatus::Reset)
    {
        return this->status;
    }

    switch (device->device.getFenceStatus(fence))
    {
    case vk::Result::eSuccess:
        return GraphicsFenceStatus::Complete;

    case vk::Result::eNotReady:
        return GraphicsFenceStatus::Submitted;

    default:
        return GraphicsFenceStatus::Reset;
    }
}

void GraphicsFence::wait(uint32_t timeout) const
{
    device->device.waitForFences({ fence }, true, timeout);
}

void GraphicsFence::reset()
{
    device->device.resetFences({ fence });
    this->status = GraphicsFenceStatus::Reset;
}
