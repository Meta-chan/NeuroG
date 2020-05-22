#version 330 core

in float varhorizontal;
in float varvertical;

uniform sampler2D weights;
uniform sampler2D nexterror;
uniform sampler2D prevvector;
uniform int prevlength;
uniform int nextlength;
uniform float koef;

out float newweightselem;

void main()
{
	int ihorizontal = int((prevlength + 1) * (varhorizontal * 0.5f + 0.5f));
	int ivertical = int(nextlength * (varvertical * 0.5f + 0.5f));
	
	float oldweightselem = texelFetch(weights, ivec2(ihorizontal, ivertical), 0).r;
	float nexterrorelem = texelFetch(nexterror, ivec2(0, ivertical), 0).r;
	if (ihorizontal != 0) nexterrorelem *= texelFetch(prevvector, ivec2(0, ihorizontal - 1), 0).r;
	//newweightselem = 0.001 * (oldweightselem + koef * nexterrorelem) - 0.42;
	newweightselem = oldweightselem + koef * nexterrorelem;
}