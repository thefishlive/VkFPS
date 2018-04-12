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
#include <vulkan/vulkan.hpp>

struct Vertex
{
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec4 color;

	Vertex(glm::vec4 position, glm::vec4 normal, glm::vec4 color = { 1, 1, 1, 1 }) : position(position), normal(normal), color(color) {}
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec4 color = { 1, 1, 1, 1 }) : position(glm::vec4(position, 1.0)), normal(glm::vec4(normal, 1.0)), color(color) {}

	static std::vector<vk::VertexInputAttributeDescription> get_vertex_input_attributes()
	{
		return std::vector<vk::VertexInputAttributeDescription>{
			vk::VertexInputAttributeDescription(
				0, 0, 
				vk::Format::eR32G32B32A32Sfloat,
				offsetof(Vertex, position)
			),
			vk::VertexInputAttributeDescription(
				1, 0,
				vk::Format::eR32G32B32A32Sfloat,
				offsetof(Vertex, normal)
			),
			vk::VertexInputAttributeDescription(
				2, 0,
				vk::Format::eR32G32B32A32Sfloat,
				offsetof(Vertex, color)
			)
		};
	}

	static std::vector<vk::VertexInputBindingDescription> get_vertex_input_bindings()
	{
		return std::vector<vk::VertexInputBindingDescription>{
			vk::VertexInputBindingDescription(
				0, sizeof(Vertex), vk::VertexInputRate::eVertex
			)
		};		
	}
};

struct MaterialShaderData
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float alpha;


	MaterialShaderData(const glm::vec4& ambient, const glm::vec4& diffuse, const glm::vec4& specular, float alpha)
		: ambient(ambient),
		  diffuse(diffuse),
		  specular(specular),
		  alpha(alpha)
	{
	}
};

struct VertexShaderData
{
	glm::mat4 model;

	explicit VertexShaderData(const glm::mat4 &model)
		: model(model)
	{
	}
};

struct CameraShaderData
{
	glm::mat4 proj_view;


	explicit CameraShaderData(const glm::mat4& proj_view)
		: proj_view(proj_view)
	{
	}
};

#define SHADER_LIGHT_COUNT 4

struct LightShaderData
{
	bool enabled[SHADER_LIGHT_COUNT];
	glm::vec4 direction[SHADER_LIGHT_COUNT];
	glm::vec4 color[SHADER_LIGHT_COUNT];
};
