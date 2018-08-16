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

#include "r_model.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "r_camera.h"
#include "r_material.h"

#include "u_debug.h"
#include "u_defines.h"
#include "u_io.h"

Model::Model(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<GraphicsDevmem>& devmem, std::shared_ptr<Renderer>& renderer, std::vector<Vertex>& verticies, std::vector<std::unique_ptr<Material>>& materials, std::vector<std::vector<uint32_t>>& indicies)
    : device(device), devmem(devmem), renderer(renderer), position(0, 0, 0)
{
    /*
     * transfer data from indicies + materials to material_data
     */
    unsigned int index = 0;

    for (unsigned int i = 0; i < materials.size(); i++)
    {
        material_data.push_back(
            std::make_unique<MaterialData>(
                materials[i],
                indicies[i],
                index
            )
        );

        index += indicies[i].size();
    }

    this->index_count = index;

    vk::BufferCreateInfo vbuf_create_info(
        vk::BufferCreateFlags(0),
        verticies.size() * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    );

    VmaAllocationCreateInfo vbuf_alloc_info{};
    vbuf_alloc_info.flags = 0;
    vbuf_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vbuf_alloc_info.pUserData = STRING_TO_DATA("Vertex Buffer");

    vertex_buffer = devmem->create_buffer(vbuf_create_info, vbuf_alloc_info);

    void *data;
    vertex_buffer->map_memory(&data);
    memcpy(data, verticies.data(), verticies.size() * sizeof(Vertex));
    vertex_buffer->unmap_memory();
    vertex_buffer->commit_memory();

    vk::BufferCreateInfo ibuf_create_info(
        vk::BufferCreateFlags(0),
        sizeof(uint32_t) * index_count,
        vk::BufferUsageFlagBits::eIndexBuffer,
        vk::SharingMode::eExclusive
    );

    VmaAllocationCreateInfo ibuf_alloc_info{};
    ibuf_alloc_info.flags = 0;
    ibuf_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    ibuf_alloc_info.pUserData = STRING_TO_DATA("Index Buffer");

    index_buffer = devmem->create_buffer(ibuf_create_info, ibuf_alloc_info);

    index_buffer->map_memory(&data);
    for (const auto & material : material_data)
    {
        memcpy(data, material->indicies.data(), material->indicies.size() * sizeof(uint32_t));
    }
    index_buffer->unmap_memory();
    index_buffer->commit_memory();

    command_buffers = renderer->alloc_render_command_buffers();
    this->invalidate_recording();
}

Model::~Model()
{
}

void Model::invalidate_recording()
{
	for (auto i = 0; i < command_buffers.size(); i++)
	{
		renderer->start_secondary_command_buffer(command_buffers[i], i, 0);

		std::vector<vk::Buffer> vbufs{ vertex_buffer->buffer };
		std::vector<VkDeviceSize> voffsets{ 0 };

		command_buffers[i].bindVertexBuffers(0, (uint32_t)vbufs.size(), vbufs.data(), voffsets.data());
		command_buffers[i].bindIndexBuffer(index_buffer->buffer, 0, vk::IndexType::eUint32);

		glm::mat4 translation = glm::translate(glm::mat4(1), position);
        glm::mat4 rotation = glm::toMat4(this->rotation);
		VertexShaderData shader_data(translation);

		for (const auto & data : material_data)
		{
			data->material->bind_material(command_buffers[i]);
			data->material->push_shader_data(command_buffers[i], 0, vk::ShaderStageFlagBits::eVertex, sizeof(VertexShaderData), &shader_data);

			command_buffers[i].drawIndexed((uint32_t)data->indicies.size(), 1, data->start_index, 0, 0);
		}

		renderer->end_secondary_command_buffer(command_buffers[i]);
	}
}

void Model::render(vk::CommandBuffer cmd, uint32_t image) const
{
	cmd.executeCommands(this->command_buffers[image]);
}
