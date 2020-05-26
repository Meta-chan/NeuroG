#version 330 core

uniform sampler2D weights;
uniform sampler2D nexterror;
uniform sampler2D prevvector;
uniform int nextlength;

out float preverrorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	float error = 0.0f;
	for (int i = 0; i < nextlength; i++)
	{
		error += texelFetch(weights, ivec2(iposition + 1, i), 0).r * texelFetch(nexterror, ivec2(0, i), 0).r;
	}
	float prevvectorelem = texelFetch(prevvector, ivec2(0, iposition), 0).r;
	preverrorelem = (1 - prevvectorelem * prevvectorelem) * error;
}