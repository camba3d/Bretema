#version 450

layout (location = 0) in vec3 fColor;

layout (location = 0) out vec4 FragColor;

layout(set = 0, binding = 1) uniform  SceneData{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} scene;

void main()
{
    FragColor = vec4(fColor, 1.0);
    FragColor = vec4(fColor + scene.ambientColor.xyz,1.0f);
}
