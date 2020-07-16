//FORWARD
const char *shader_fragment_forward = 
R"(#version 330 core

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
};)";

//LASTBACKWARD
const char *shader_fragment_lastbackward_bitwise = 
R"(#version 330 core

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
};)";

const char *shader_fragment_lastbackward =
R"(#version 330 core

uniform sampler2D lastvector;
uniform sampler2D goal;

out float lasterrorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	float goalelem = texelFetch(goal, ivec2(0, iposition), 0).r;
	float lastvectorelem = texelFetch(lastvector, ivec2(0, iposition), 0).r;
	lasterrorelem = (1 - lastvectorelem * lastvectorelem) * (goalelem - lastvectorelem);
};)";

//BACKWARD
const char *shader_fragment_backward =
R"(#version 330 core
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
};)";

//CORRIGATE
const char *shader_fragment_corrigate = 
R"(#version 330 core

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
};)";

//BITWISE 1D
const char *shader_fragment_bitwise_1d =
R"(#version 330 core

uniform sampler2D vector;

out float outvectorelem;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	vec4 invectorelem = 255.0f * texelFetch(vector, ivec2(0, iposition), 0);
	outvectorelem = uintBitsToFloat((uint(invectorelem.a) << 24)	|
									(uint(invectorelem.b) << 16)	|
									(uint(invectorelem.g) << 8)		|
									(uint(invectorelem.r)));
};)";

//BITWISE 2D
const char *shader_fragment_bitwise_2d =
R"(#version 330 core

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
};)";

//DISTRIBUTE 1D
const char *shader_vertex_distribute_1d =
R"(#version 330 core
layout(location = 0) in float position;

void main()
{
	gl_Position = vec4(0.01f, position, 0.0f, 1.0f);
	//WTF? Does not work with 0.0f
};)";

//DISTRIBUTE 2D
const char *shader_vertex_distribute_2d = 
R"(#version 330 core
layout(location = 0) in float vertical;
layout(location = 1) in float horizontal;

void main()
{
	gl_Position = vec4(horizontal, vertical, 0.0f, 1.0f);
};)";