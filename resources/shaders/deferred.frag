#version 450
#extension GL_ARB_separate_shader_objects : enable

const int lightcount = 4;
const int pcoffset = 64;
const float ambient_light = 0.0f;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec2 in_uv;

layout(binding = 2) uniform sampler2D samplerAlbedo;
layout(binding = 3) uniform sampler2D samplerPosition;
layout(binding = 4) uniform sampler2D samplerNormalMap;

layout(location = 0) out vec4 out_color;

struct Light {
    vec4 direction;
    vec4 color;
    float radius;
};

layout(binding = 1) uniform LightData {
	Light lights[lightcount];
} light_data;

void main() {
	vec3 direction = vec3(1.0f, 1.0f, 1.0f);
	vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    vec3 normal = texture(samplerNormalMap, in_uv).xyz;
    vec4 position = texture(samplerPosition, in_uv).xyzw;
    vec4 albedo = texture(samplerAlbedo, in_uv).rgba;

	float intensity = min(max(dot(normal, direction), 0.0), 1.0);
	vec4 fragColor = vec4(0, 0, 0, 0);
    fragColor += ambient_light * color;
	fragColor += intensity * color;
	fragColor = clamp(fragColor, 0.0, 1.0);

	out_color = vec4(fragColor.rgb, 1.0);
    //out_color = texture(samplerNormalMap, in_uv).rgba;
}
