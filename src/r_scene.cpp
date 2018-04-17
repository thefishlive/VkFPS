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

#include "r_scene.h"

std::shared_ptr<Scene> Scene::current_scene;

Scene::Scene(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<GraphicsDevmem>& devmem)
	: device(device), devmem(devmem)
{
	vk::BufferCreateInfo buffer_create_info(
		vk::BufferCreateFlags(0),
		sizeof(LightShaderData),
		vk::BufferUsageFlagBits::eUniformBuffer
	);

	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.flags = 0;
	alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	light_data_buffer = devmem->create_buffer(buffer_create_info, alloc_create_info);

	light_data.enabled[0] = true;
	light_data.color[0] = glm::vec4(0.0f, 1.0f, 1.0f, 0.0f);
	light_data.direction[0] = glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);

	update_buffer();
}

Scene::~Scene()
{
}

vk::DescriptorBufferInfo Scene::get_light_data_info() const
{
	return vk::DescriptorBufferInfo(
		light_data_buffer->buffer,
		0, sizeof(LightShaderData)
	);
}

void Scene::update_buffer() const
{
	void *data;
	light_data_buffer->map_memory(&data);
	memcpy(data, &light_data, sizeof(light_data));
	light_data_buffer->unmap_memory();
}
