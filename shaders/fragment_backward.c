#version 330 core

in float varposition;

uniform sampler2D weights;
uniform sampler2D nexterror;
uniform sampler2D prevvector;
uniform int prevlength;
uniform int nextlength;

out float preverrorelem;

void main()
{
	int iposition = int(prevlength * (varposition * 0.5f + 0.5f));
	float error = 0.0f;
	for (int i = 0; i < nextlength; i++)
	{
		error += texelFetch(weights, ivec2(iposition + 1, i), 0).r * texelFetch(nexterror, ivec2(0, i), 0).r;
	}
	float prevvectorelem = texelFetch(prevvector, ivec2(0, iposition), 0).r;
	preverrorelem = (1 - prevvectorelem * prevvectorelem) * error;
}