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
#include "r_model.h"
#include <tiny_obj_loader.h>
#include "r_image_loader.h"

class ModelLoader
{
public:
    ModelLoader(std::shared_ptr<GraphicsDevice> &device, std::shared_ptr<GraphicsDevmem> &devmem, std::shared_ptr<Renderer> &renderer);
    ~ModelLoader();

    std::unique_ptr<Model> ModelLoader::load_model(std::string path);

private:
    std::shared_ptr<GraphicsDevice> device;
    std::shared_ptr<GraphicsDevmem> devmem;
    std::shared_ptr<Renderer> renderer;

    std::unique_ptr<RenderImageLoader> image_loader;

    std::shared_ptr<GraphicsDevmemImage> dummy_image;

    static std::string get_library_path(const std::string& library, const std::string& file);

    static glm::vec4 to_vec4(float f[3]);

    static glm::vec4 get_vertex(const tinyobj::attrib_t& attrib, int index);
    static glm::vec4 get_normal(const tinyobj::attrib_t& attrib, int index);
    static glm::vec2 get_uv(const tinyobj::attrib_t& attrib, int index);

    void create_dummy_texture_sampler();
};
