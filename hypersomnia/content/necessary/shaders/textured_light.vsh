#version 300 es
precision mediump int;
precision mediump float;

uniform mat4 projection_matrix;
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;

smooth out vec4 theColor;
out vec2 theTexcoord;

void main() 
{
	vec4 output_vert;
	output_vert.x = position.x;		
	output_vert.y = position.y;				
	output_vert.z = 0.0f;						
	output_vert.w = 1.0f;
	
	gl_Position = projection_matrix*output_vert;
	theColor = color;
	theTexcoord = texcoord;
}