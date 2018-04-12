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

#include <glm/glm.hpp>
#include <map>
#include <vulkan/vulkan.hpp>

#include "g_device.h"
#include "g_devmem.h"
#include "g_renderpass.h"
#include "r_material.h"
#include "r_shaderif.h"

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
	Model(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<GraphicsDevmem>& devmem, GraphicsRenderpass & renderpass, std::string file);
	~Model();

	void set_position(glm::vec3 & position) { this->position = position; }

	void render(vk::CommandBuffer command_buffer);

private:
	std::shared_ptr<GraphicsDevice>& device;
	std::shared_ptr<GraphicsDevmem> devmem;
	GraphicsRenderpass renderpass;

	glm::vec3 position;

	std::vector<Vertex> verticies;
	std::map<std::string, std::unique_ptr<MaterialData>> materials;

	uint32_t index_count;

	GraphicsDevmemBuffer *vertex_buffer;
	GraphicsDevmemBuffer *index_buffer;

	void load_material_lib(const std::string& library_file);
	void load_model_data(std::string file);
	static std::string get_library_path(const std::string& library, const std::string& file);
};
