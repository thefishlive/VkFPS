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

#include "g_devmem.h"
#include "r_shaderif.h"

class Camera
{
public:
	Camera(std::shared_ptr<GraphicsDevmem> devmem, float fov, float aspect_ratio, float near, float far, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

	~Camera();
	
	glm::mat4 get_matrix() const;
	vk::DescriptorBufferInfo get_buffer_info() const;

	static std::shared_ptr<Camera> get();
	static void set(std::shared_ptr<Camera>& camera);
	
private:
	float fov;
	float aspect_ratio;
	float near;
	float far;

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	CameraShaderData shader_data;
	GraphicsDevmemBuffer *shader_data_buffer;

	static std::shared_ptr<Camera> current_camera;

	void update_buffer() const;
};
