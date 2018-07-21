
#include "r_camera.h"

#include <glm/gtc/matrix_transform.hpp>

std::shared_ptr<Camera> Camera::current_camera;

Camera::Camera(
    std::shared_ptr<GraphicsDevmem> devmem, 
    float fov, float aspect_ratio, float near, float far, 
    const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : fov(fov), aspect_ratio(aspect_ratio), near(near), far(far),
      position(position), target(target), up(up),  shader_data(get_matrix())
{
	vk::BufferCreateInfo buffer_create_info(
		vk::BufferCreateFlags(0),
		sizeof(CameraShaderData),
		vk::BufferUsageFlagBits::eUniformBuffer
	);

	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.flags = 0;
	alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; /* Try for both DEVICE_LOCAL and HOST_VISIBLE (AMD), if not fallback to HOST_VISIBLE */

	shader_data_buffer = devmem->create_buffer(buffer_create_info, alloc_create_info);

	update_buffer();
}

Camera::~Camera()
{
}

glm::mat4 Camera::get_matrix() const
{
    glm::mat4 view = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 proj = glm::perspective(fov, aspect_ratio, near, far);
	return proj * view;
}

vk::DescriptorBufferInfo Camera::get_buffer_info() const
{
	return vk::DescriptorBufferInfo(
		shader_data_buffer->buffer,
		0, sizeof(CameraShaderData)
	);
}

void Camera::move(glm::vec3 vec)
{
    position += vec;
    target += vec;
    update_buffer();
}

std::shared_ptr<Camera> Camera::get()
{
	return current_camera;
}

void Camera::set(std::shared_ptr<Camera>& camera)
{
	current_camera = camera;
}

void Camera::update_buffer() const
{
	glm::mat4 proj_view = get_matrix();

	void *data;
	shader_data_buffer->map_memory(&data);
	memcpy(data, &proj_view, sizeof(proj_view));
	shader_data_buffer->unmap_memory();
}
