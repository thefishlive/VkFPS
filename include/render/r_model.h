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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vulkan/vulkan.hpp>
#include <vector>

#include "g_device.h"
#include "g_devmem.h"

#include "r_material.h"
#include "r_renderer.h"
#include "g_shaderif.h"

struct MaterialData
{
	std::unique_ptr<Material> material;
	std::vector<uint32_t> indicies;
	uint32_t start_index;

	explicit MaterialData(std::unique_ptr<Material> & material, const std::vector<uint32_t>& indicies = std::vector<uint32_t>(), uint32_t start_index = 0)
		: material(std::move(material)),
		indicies(indicies),
		start_index(start_index)
	{
	}
};

class Model
{
public:
	Model(
        std::shared_ptr<GraphicsDevice>& device,
        std::shared_ptr<GraphicsDevmem>& devmem,
        std::shared_ptr<Renderer>& renderer,
        std::vector<Vertex>& verticies,
        std::vector<std::unique_ptr<Material>>& materials,
        std::vector<std::vector<uint32_t>>& indicies
        );
	~Model();

    void set_position(glm::vec3 & position) { this->position = position; }
    void set_rotation(glm::quat & rotation) { this->rotation = rotation; }

	void invalidate_recording();
	void render(vk::CommandBuffer command_buffer, uint32_t image) const;

private:
	std::shared_ptr<GraphicsDevice>& device;
	std::shared_ptr<GraphicsDevmem> devmem;
	std::shared_ptr<Renderer> renderer;

	std::vector<vk::CommandBuffer> command_buffers;

	glm::vec3 position;
    glm::quat rotation;

	std::vector<Vertex> verticies;
	std::vector<std::unique_ptr<MaterialData>> material_data;

	uint32_t index_count;

	std::unique_ptr<GraphicsDevmemBuffer> vertex_buffer;
	std::unique_ptr<GraphicsDevmemBuffer> index_buffer;
};
