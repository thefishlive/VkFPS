#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 frag_color;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(position);
    frag_color = color;
}
