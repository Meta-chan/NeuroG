#version 330 core

uniform sampler2D weights;

out float outweightselem;

void main()
{
	int ihorizontal = int(gl_FragCoord.x - 0.5);
	int ivertical = int(gl_FragCoord.y - 0.5);
	vec4 inweightselem = 255.0f * texelFetch(weights, ivec2(ihorizontal, ivertical), 0);
	outweightselem = uintBitsToFloat((uint(inweightselem.a) << 24)	|
									(uint(inweightselem.b) << 16)	|
									(uint(inweightselem.g) << 8)	|
									(uint(inweightselem.r)));
	//outweightselem = 0.42 + 0.001 * outweightselem;
}