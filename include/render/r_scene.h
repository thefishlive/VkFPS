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

#include <memory>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "g_device.h"
#include "g_devmem.h"
#include "g_shaderif.h"
#include "r_model.h"

class Scene
{
public:
	Scene(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<GraphicsDevmem>& devmem);
	~Scene();

	// TODO register models to a scene

	vk::DescriptorBufferInfo get_light_data_info() const;
    void add_model(std::unique_ptr<Model> model);

    void render_models(vk::CommandBuffer buffer, uint32_t index);

	static std::shared_ptr<Scene> get() { return current_scene; }
	static void set(std::shared_ptr<Scene>& scene) { current_scene = scene; }

private:
	std::shared_ptr<GraphicsDevice> device;
	std::shared_ptr<GraphicsDevmem> devmem;

	LightShaderData light_data;
	std::unique_ptr<GraphicsDevmemBuffer> light_data_buffer;

    std::vector<std::unique_ptr<Model>> models;

	static std::shared_ptr<Scene> current_scene;

	void update_buffer() const;
};
