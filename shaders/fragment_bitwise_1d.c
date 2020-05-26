#version 330 core

uniform sampler2D vector;

out float outvectorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	vec4 invectorelem = 255.0f * texelFetch(vector, ivec2(0, iposition), 0);
	outvectorelem = uintBitsToFloat((uint(invectorelem.a) << 24)	|
									(uint(invectorelem.b) << 16)	|
									(uint(invectorelem.g) << 8)		|
									(uint(invectorelem.r)));
}