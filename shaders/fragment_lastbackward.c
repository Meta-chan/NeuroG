#version 330 core

in float varposition;

uniform sampler2D lastvector;
uniform sampler2D goal;
uniform int lastlength;

out float lasterrorelem;

void main()
{
	int iposition = int(lastlength * (varposition * 0.5f + 0.5f));
	vec4 invectorelem = 255.0f * texelFetch(goal, ivec2(0, iposition), 0);
	float goalelem = uintBitsToFloat((uint(invectorelem.a) << 24)	|
									(uint(invectorelem.b) << 16)	|
									(uint(invectorelem.g) << 8)		|
									(uint(invectorelem.r)));
	float lastvectorelem = texelFetch(lastvector, ivec2(0, iposition), 0).r;
	lasterrorelem = (1 - lastvectorelem * lastvectorelem) * (goalelem - lastvectorelem);
}