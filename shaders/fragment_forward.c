#version 330 core

in float varposition;

uniform sampler2D weights;
uniform sampler2D prevvector;
uniform int prevlength;
uniform int nextlength;

out float nextvectorelem;

void main()
{
	int iposition = int(nextlength * (varposition * 0.5f + 0.5f));
	float sum = texelFetch(weights, ivec2(0, iposition), 0).r;
	for (int i = 0; i < prevlength; i++)
	{
		sum += texelFetch(weights, ivec2(i + 1, iposition), 0).r * texelFetch(prevvector, ivec2(0, i), 0).r;
	}
	nextvectorelem = tanh(sum);
}