#version 330 core

in float varhorizontal;
in float varvertical;

uniform sampler2D weights;
uniform int height;
uniform int width;

out float outweightselem;

void main()
{
	int ihorizontal = int(width * (varhorizontal * 0.5f + 0.5f));
	int ivertical = int(height * (varvertical * 0.5f + 0.5f));
	vec4 inweightselem = 255.0f * texelFetch(weights, ivec2(ihorizontal, ivertical), 0);
	outweightselem = uintBitsToFloat((uint(inweightselem.a) << 24)	|
									(uint(inweightselem.b) << 16)	|
									(uint(inweightselem.g) << 8)	|
									(uint(inweightselem.r)));
}