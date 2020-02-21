#version 420

layout(std140, binding = 1) uniform Time
{
	uniform float uTime;
};

layout(binding = 0) uniform samplerCube uTexCube;

struct vData
{
	vec2 texcoord;
	vec3 norm;
	vec3 pos;
};
layout(location = 0) in vData o;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outEmissive;
layout (location = 3) out float outRoughness;
layout (location = 4) out float outMetallic;

void main()
{
	outEmissive = texture(uTexCube, normalize(o.norm)).rgb;
}
