#version 450
#extension GL_ARB_separate_shader_objects : enable

const int lightcount = 4;
const int pcoffset = 64;

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec4 normal;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform LightData {
	bool enabled[lightcount];
	vec4 direction[lightcount];
	vec4 color[lightcount];
} light_data;

layout(push_constant) uniform ShaderData {
	layout(offset=pcoffset) vec4 ambient;
	layout(offset=pcoffset+16) vec4 diffuse;
	layout(offset=pcoffset+32) vec4 specular;
	layout(offset=pcoffset+48) float alpha;
} shader_data;

void main() {
	vec4 direction = vec4(-1.0f, -1.0f, -1.0f, 1.0f);
	vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	float intensity = max(dot(normal, direction), 0.0);
	vec4 fragColor = shader_data.ambient;
	fragColor += intensity * color;
	fragColor = clamp(fragColor, 0.0, 1.0);

	out_color = vec4(fragColor.rgb, 1.0);
}
