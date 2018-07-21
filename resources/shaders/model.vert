#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

const int pcoffset = 0;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec2 out_uv;

layout(binding = 0) uniform CameraData {
	mat4 proj_view;
} camera_data;

layout(push_constant) uniform ModelData {
	layout(offset=pcoffset) mat4 model;
} model_data;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    out_position = model_data.model * vec4(position);
    out_position.y = -out_position.y;

    out_normal = transpose(inverse(model_data.model)) * normalize(normal);
    
    gl_Position = camera_data.proj_view * out_position;
    out_uv = in_uv;
}
