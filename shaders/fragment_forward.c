#version 330 core

uniform sampler2D weights;
uniform sampler2D prevvector;
uniform int prevlength;

out float nextvectorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	float sum = texelFetch(weights, ivec2(0, iposition), 0).r;
	for (int i = 0; i < prevlength; i++)
	{
		sum += texelFetch(weights, ivec2(i + 1, iposition), 0).r * texelFetch(prevvector, ivec2(0, i), 0).r;
	}

	nextvectorelem = tanh(sum);
	//nextvectorelem = 0.001 * tanh(sum) - 0.42;
};