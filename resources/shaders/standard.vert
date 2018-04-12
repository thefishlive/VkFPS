#version 450
#extension GL_ARB_separate_shader_objects : enable

const int pcoffset = 0;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

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
    gl_Position = camera_data.proj_view * model_data.model * vec4(position);
    out_normal = model_data.model * normal;
    out_color = color;
}
