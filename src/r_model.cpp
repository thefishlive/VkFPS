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

Model::Model(std::shared_ptr<GraphicsDevice>& device, std::shared_ptr<GraphicsDevmem>& devmem, std::shared_ptr<Renderer>& renderer, std::string file)
	: device(device), devmem(devmem), renderer(renderer), position(0, 0, 0)
{
	load_model_data(file);

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
	for (const auto & material : materials)
	{
		memcpy(data, material.second->indicies.data(), material.second->indicies.size() * sizeof(uint32_t));
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

		for (const auto & data : materials)
		{
			data.second->material->bind_material(command_buffers[i]);
			data.second->material->push_shader_data(command_buffers[i], 0, vk::ShaderStageFlagBits::eVertex, sizeof(VertexShaderData), &shader_data);

			command_buffers[i].drawIndexed((uint32_t)data.second->indicies.size(), 1, data.second->start_index, 0, 0);
		}

		renderer->end_secondary_command_buffer(command_buffers[i]);
	}
}

void Model::render(vk::CommandBuffer cmd, uint32_t image) const
{
	cmd.executeCommands(this->command_buffers[image]);
}

struct FaceVertexData
{
	int vertex;
	int uv;
	int normal;

	FaceVertexData(int vertex, int uv, int normal) : vertex(vertex), uv(uv), normal(normal) {}

	FaceVertexData(std::string str);
};

FaceVertexData::FaceVertexData(std::string str)
{
	uv = 0;
	sscanf(str.c_str(), "%d//%d", &vertex, &normal);
}

void Model::load_material_lib(const std::string& library_file)
{
	LOG_INFO("Loading material file %s", FILENAME_TO_PATH(library_file).c_str());
	std::ifstream fstream(FILENAME_TO_PATH(library_file));

	std::string mat_name;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float alpha = 0.0f;

	std::string line;
	while (std::getline(fstream, line))
	{
		std::istringstream lstream(line);
		std::string type;

		if (line == "")
		{
			continue;
		}

		if (!(lstream >> type))
		{
			break;
		}

		if (type == "#")
		{
			continue;
		}
		else if (type == "Ka")
		{
			float r, g, b;
			if (!(lstream >> r >> g >> b))
			{
				LOG_ERROR("Error parsing line in mtl file of type AMBIENT (%s)", line);
				break;
			}
			ambient = glm::vec3(r, g, b);
		}
		else if (type == "Kd")
		{
			float r, g, b;
			if (!(lstream >> r >> g >> b))
			{
				LOG_ERROR("Error parsing line in mtl file of type DIFFUSE (%s)", line);
				break;
			}
			diffuse = glm::vec3(r, g, b);
		}
		else if (type == "Ks")
		{
			float r, g, b;
			if (!(lstream >> r >> g >> b))
			{
				LOG_ERROR("Error parsing line in mtl file of type SPECULAR (%s)", line);
				break;
			}
			specular = glm::vec3(r, g, b);
		}
		else if (type == "d")
		{
			float a;
			if (!(lstream >> a))
			{
				LOG_ERROR("Error parsing line in mtl file of type ALPHA (%s)", line);
				break;
			}

			alpha = a;
		}
		else if (type == "newmtl")
		{
			std::string name;
			if (!(lstream >> name))
			{
				LOG_ERROR("Error parsing line in mtl file of type NEWMTL (%s)", line);
				break;
			}

			if (mat_name != "")
			{
				std::unique_ptr<Material> material = std::make_unique<Material>(device, renderer, glm::vec4(ambient, 1.0f), glm::vec4(diffuse, 1.0f), glm::vec4(specular, 1.0f), alpha);
				materials.emplace(mat_name, std::make_unique<MaterialData>(material));

				alpha = 0.0f;
				ambient = glm::vec3();
				diffuse = glm::vec3();
				specular = glm::vec3();
			}

			mat_name = name;
			
		}
	}

	if (mat_name != "")
	{
		std::unique_ptr<Material> material = std::make_unique<Material>(device, renderer, glm::vec4(ambient, 1.0f), glm::vec4(diffuse, 1.0f), glm::vec4(specular, 1.0f), alpha);
		materials.emplace(mat_name, std::make_unique<MaterialData>(material));
	}
}

void Model::load_model_data(std::string file)
{
	LOG_INFO("Loading model file %s", FILENAME_TO_PATH(file).c_str());

	std::ifstream fstream(FILENAME_TO_PATH(file));
	std::vector<glm::vec3> verticies;
	std::vector<glm::vec3> normals;
	std::map<std::string, std::vector<std::array<FaceVertexData, 3>>> faces;
	std::vector<std::array<FaceVertexData, 3>> current_faces;
	std::string current_material;

	if (fstream.bad())
	{
		std::string message = "Could not open file " + file;
		throw std::exception(message.c_str());
	}

	std::string line;
	while (std::getline(fstream, line))
	{
		std::istringstream lstream(line);
		std::string type;

		if (line == "")
		{
			continue;
		}

		if (!(lstream >> type))
		{
			break;
		}

		if (type == "#")
		{
			continue;
		}
		else if (type == "v")
		{
			float x, y, z;
			if (!(lstream >> x >> y >> z))
			{
				LOG_ERROR("Error parsing line in obj file of type VERTEX (%s)", line);
				break;
			}

			verticies.push_back({ x, y, z });
		}
		else if (type == "vn")
		{
			float x, y, z;
			if (!(lstream >> x >> y >> z))
			{
				LOG_ERROR("Error parsing line in obj file of type NORMAL (%s)", line);
				break;
			}

			normals.push_back({ x, y, z });	
		}
		else if (type == "f")
		{
			std::string f1, f2, f3;
			if (!(lstream >> f1 >> f2 >> f3))
			{
				LOG_ERROR("Error parsing line in obj file of type FACE (%s)", line);
				break;
			}

			current_faces.push_back(std::array<FaceVertexData, 3>{
				FaceVertexData(f1), 
				FaceVertexData(f2), 
				FaceVertexData(f3) 
			});
		}
		else if (type == "mtllib")
		{
			std::string library_file;
			if(!(lstream >> library_file))
			{
				LOG_ERROR("Error parsing line in obj file of type MTLLIB (%s)", line);
				break;
			}

			load_material_lib(get_library_path(library_file, file));
		}
		else if (type == "usemtl")
		{
			std::string material;
			if (!(lstream >> material))
			{
				LOG_ERROR("Error parsing line in obj file of type USEMTL (%s)", line);
				break;
			}

			if (current_material != "")
			{
				faces.emplace(current_material, current_faces);
			}

			current_material = material;
		}
		else
		{
			// skip line, unsupported type
			continue;
		}
	}

	if (current_material != "")
	{
		faces.emplace(current_material, current_faces);
	}

	int i = 0;

	std::vector<uint32_t> indicies;

	for (const auto & face : faces)
	{
		materials.at(face.first)->start_index = i;

		for (const auto & curr : face.second)
		{
			this->verticies.push_back(Vertex(
				verticies[curr[0].vertex - 1],
				normals[curr[0].normal - 1]
			));
			this->verticies.push_back(Vertex(
				verticies[curr[1].vertex - 1],
				normals[curr[1].normal - 1]
			));
			this->verticies.push_back(Vertex(
				verticies[curr[2].vertex - 1],
				normals[curr[2].normal - 1]
			));

			indicies.push_back(i);
			indicies.push_back(i + 1);
			indicies.push_back(i + 2);

			i += 3;
		}

		materials.at(face.first)->indicies = std::vector<uint32_t>(indicies);
		indicies.clear();
	}

	this->index_count = i;
}

std::string Model::get_library_path(const std::string& library, const std::string& file)
{
	return file.substr(0, file.find_last_of('/') + 1) + library;
}
