#ifndef MATHG_SOURCE
#define MATHG_SOURCE

const char *MathG::Source::_distribute1d =
R"(#version 330 core
layout(location = 0) in float position;

void main()
{
	gl_Position = vec4(0.5f, position, 0.0f, 1.0f);
};)";

const char *MathG::Source::_distribute2d =
R"(#version 330 core
layout(location = 0) in float vertical;
layout(location = 1) in float horizontal;

void main()
{
	gl_Position = vec4(horizontal, vertical, 0.0f, 1.0f);
};)";

const char *MathG::Source::_add = 
R"(#version 330 core

uniform sampler2D a;
uniform sampler2D b;

out float c;

void main()
{
	int iposition = int(gl_FragCoord.y - 0.5);
	c = texelFetch(a, ivec2(0, iposition), 0).r + texelFetch(b, ivec2(0, iposition), 0).r;
};)";

#endif	//#ifndef MATHG_SOURCE