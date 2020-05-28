#version 330 core

uniform sampler2D weights;
uniform sampler2D nexterror;
uniform sampler2D prevvector;
uniform float coefficient;

out float newweightselem;

void main()
{
	int ihorizontal = int(gl_FragCoord.x - 0.5);
	int ivertical = int(gl_FragCoord.y - 0.5);
	
	float oldweightselem = texelFetch(weights, ivec2(ihorizontal, ivertical), 0).r;
	float nexterrorelem = texelFetch(nexterror, ivec2(0, ivertical), 0).r;
	if (ihorizontal != 0) nexterrorelem *= texelFetch(prevvector, ivec2(0, ihorizontal - 1), 0).r;
	newweightselem = oldweightselem + coefficient * nexterrorelem;
	//newweightselem = 0.001 * (oldweightselem + coefficient * nexterrorelem) - 0.42;
};