#version 450
#extension GL_ARB_separate_shader_objects : enable

const int pcoffset = 64;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_final;
layout(location = 1) out vec4 out_color;
layout(location = 2) out vec4 out_position;
layout(location = 3) out vec4 out_normal;

layout(binding = 2) uniform sampler2D ambient_texture;
layout(binding = 3) uniform sampler2D diffuse_texture;
layout(binding = 4) uniform sampler2D specular_texture;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 256.0f;

layout(push_constant) uniform ShaderData {
	layout(offset=pcoffset) vec4 ambient;
	layout(offset=pcoffset+16) vec4 diffuse;
	layout(offset=pcoffset+32) vec4 specular;
	layout(offset=pcoffset+48) float alpha;
} shader_data;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() {
	out_position = in_position;
	out_position.a = linearDepth(gl_FragCoord.z);

	out_normal = normalize(in_normal);

    out_color = texture(diffuse_texture, in_uv);

	// Avoid validation error
	// out_final = vec4(0);
}
