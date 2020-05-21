#version 330 core
layout(location = 0) in float position;

out float varposition;

void main()
{
	varposition = position;
	gl_Position = vec4(0.01f, position, 0.0f, 1.0f);
	//WTF? Does not work with 0.0f
}