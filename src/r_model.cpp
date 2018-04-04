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

#include "r_material.h"
#include "u_io.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

Model::Model(std::shared_ptr<GraphicsDevice>& device, GraphicsDevmem & devmem, GraphicsRenderpass & renderpass, std::string file)
	: device(device), devmem(devmem), renderpass(renderpass)
{
	load_model_data(file);

	vk::BufferCreateInfo vbuf_create_info(
		vk::BufferCreateFlags(0),
		verticies.size() * sizeof(Vertex),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive
	);

	VmaAllocationCreateInfo vbuf_alloc_info{};
	vbuf_alloc_info.flags = 0;
	vbuf_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vertex_buffer = devmem.create_buffer(vbuf_create_info, vbuf_alloc_info);
	
	void *data;
	vertex_buffer->map_buffer(&data);
	memcpy(data, verticies.data(), verticies.size() * sizeof(Vertex));
	vertex_buffer->unmap_buffer();

	vk::BufferCreateInfo ibuf_create_info(
		vk::BufferCreateFlags(0),
		sizeof(uint32_t) * index_count,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive
	);

	VmaAllocationCreateInfo ibuf_alloc_info{};
	ibuf_alloc_info.flags = 0;
	ibuf_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	index_buffer = devmem.create_buffer(ibuf_create_info, ibuf_alloc_info);

	index_buffer->map_buffer(&data);
	for (const auto & material : materials)
	{
		memcpy(data, material.second->indicies.data(), material.second->indicies.size() * sizeof(uint32_t));
	}
	index_buffer->unmap_buffer();
}

Model::~Model()
{
	delete vertex_buffer;
	delete index_buffer;
}

void Model::render(vk::CommandBuffer cmd)
{
	std::vector<vk::Buffer> vbufs = { vertex_buffer->buffer };
	std::vector<VkDeviceSize> voffsets = { 0 };

	cmd.bindVertexBuffers(0, (uint32_t) vbufs.size(), vbufs.data(), voffsets.data());
	cmd.bindIndexBuffer(index_buffer->buffer, 0, vk::IndexType::eUint32);

	for (const auto & data : materials)
	{
		data.second->material->bind_material(cmd);

		cmd.drawIndexed((uint32_t)data.second->indicies.size(), 1, data.second->start_index, 0, 0);
	}
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
	// FIXME pathify the library_file name
#ifndef NDEBUG
	std::cout << "Loading material file " << FILENAME_TO_PATH(library_file) << std::endl;
#endif
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
#ifndef NDEBUG
				std::cerr << "Error parsing line in mtl file of type AMBIENT (" << line << ")" << std::endl;
#endif
				break;
			}
			ambient = glm::vec3(r, g, b);
		}
		else if (type == "Kd")
		{
			float r, g, b;
			if (!(lstream >> r >> g >> b))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in mtl file of type DIFFUSE (" << line << ")" << std::endl;
#endif
				break;
			}
			diffuse = glm::vec3(r, g, b);
		}
		else if (type == "Ks")
		{
			float r, g, b;
			if (!(lstream >> r >> g >> b))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in mtl file of type SPECULAR (" << line << ")" << std::endl;
#endif
				break;
			}
			specular = glm::vec3(r, g, b);
		}
		else if (type == "d")
		{
			float a;
			if (!(lstream >> a))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in mtl file of type ALPHA (" << line << ")" << std::endl;
#endif
				break;
			}

			alpha = a;
		}
		else if (type == "newmtl")
		{
			std::string name;
			if (!(lstream >> name))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in mtl file of type NEWMTL (" << line << ")" << std::endl;
#endif
				break;
			}

			if (mat_name != "")
			{
				std::unique_ptr<Material> material = std::make_unique<Material>(device, renderpass, ambient, diffuse, specular, alpha);
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
		std::unique_ptr<Material> material = std::make_unique<Material>(device, renderpass, ambient, diffuse, specular, alpha);
		materials.emplace(mat_name, std::make_unique<MaterialData>(material));
	}
}

void Model::load_model_data(std::string file)
{
#ifndef NDEBUG
	std::cout << "Loading model file " << FILENAME_TO_PATH(file) << std::endl;
#endif
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
#ifndef NDEBUG
				std::cerr << "Error parsing line in obj file of type VERTEX (" << line << ")" << std::endl;
#endif
				break;
			}

			verticies.push_back({ x, y, z });
		}
		else if (type == "vn")
		{
			float x, y, z;
			if (!(lstream >> x >> y >> z))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in obj file of type NORMAL (" << line << ")" << std::endl;
#endif
				break;
			}

			normals.push_back({ x, y, z });	
		}
		else if (type == "f")
		{
			std::string f1, f2, f3;
			if (!(lstream >> f1 >> f2 >> f3))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in obj file of type FACE (" << line << ")" << std::endl;
#endif
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
#ifndef NDEBUG
				std::cerr << "Error parsing line in obj file of type MTLLIB (" << line << ")" << std::endl;
#endif
				break;
			}

			load_material_lib(get_library_path(library_file, file));
		}
		else if (type == "usemtl")
		{
			std::string material;
			if (!(lstream >> material))
			{
#ifndef NDEBUG
				std::cerr << "Error parsing line in obj file of type USEMTL (" << line << ")" << std::endl;
#endif
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

	faces.emplace(current_material, current_faces);

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
