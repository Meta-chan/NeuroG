#version 330 core

uniform sampler2D lastvector;
uniform sampler2D goal;

out float lasterrorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	vec4 invectorelem = 255.0f * texelFetch(goal, ivec2(0, iposition), 0);
	float goalelem = uintBitsToFloat((uint(invectorelem.a) << 24)	|
									(uint(invectorelem.b) << 16)	|
									(uint(invectorelem.g) << 8)		|
									(uint(invectorelem.r)));
	float lastvectorelem = texelFetch(lastvector, ivec2(0, iposition), 0).r;
	lasterrorelem = (1 - lastvectorelem * lastvectorelem) * (goalelem - lastvectorelem);
	//lasterrorelem = 0.001 * lasterrorelem + lastvectorelem;
}