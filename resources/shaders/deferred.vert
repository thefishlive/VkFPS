#version 450
#extension GL_ARB_separate_shader_objects : enable

const int pcoffset = 0;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec2 out_uv;

layout(binding = 0) uniform CameraData {
	mat4 proj_view;
} camera_data;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(in_position);
    out_uv = in_uv;
}