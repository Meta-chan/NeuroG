#version 330 core

in float varposition;

uniform sampler2D vector;
uniform int length;

out float outvectorelem;

void main()
{
	int iposition = int(length * (varposition * 0.5f + 0.5f));
	vec4 invectorelem = 255.0f * texelFetch(vector, ivec2(0, iposition), 0);
	outvectorelem = uintBitsToFloat((uint(invectorelem.a) << 24)	|
									(uint(invectorelem.b) << 16)	|
									(uint(invectorelem.g) << 8)		|
									(uint(invectorelem.r)));
}