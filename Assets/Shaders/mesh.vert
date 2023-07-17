#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV0;
layout (location = 2) in vec3 vNormal;
layout (location = 3) in vec4 vTanget;

layout (location = 0) out vec3 fColor;

layout(set = 0, binding = 0) uniform Camera
{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} uCam;

layout(push_constant) uniform Constants
{
	mat4 normal;
	mat4 model;
} uConsts;

void main()
{
	mat4 MVP = uCam.viewproj * uConsts.model;
	gl_Position = MVP * vec4(vPosition, 1.0);
	fColor = vNormal;
}
