#version 450

layout (location = 0) out vec3 outColor;

void main()
{
	const vec3 verticesPos[3] = vec3[3](
		vec3( 1.0, 1.0, 0.0),
		vec3(-1.0, 1.0, 0.0),
		vec3( 0.0,-1.0, 0.0)
	);

	const vec3 verticesColor[3] = vec3[3](
		vec3(1.0, 0.0, 0.0), //red
		vec3(0.0, 1.0, 0.0), //green
		vec3(0.0, 0.0, 1.0)  //blue
	);

	gl_Position = vec4(verticesPos[gl_VertexIndex], 1.0);
	outColor = verticesColor[gl_VertexIndex];
}