#version 330 core
layout(location = 0) in float vertical;
layout(location = 1) in float horizontal;

out float varvertical;
out float varhorizontal;

void main()
{
	varvertical = vertical;
	varhorizontal = horizontal;
	gl_Position = vec4(horizontal, vertical, 0.0f, 1.0f);
}