#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 UV;
uniform mat4 MVP;
void main() 
{
	gl_Position = vec4(position, 0.0, 1.0);
	UV = texCoord;
}