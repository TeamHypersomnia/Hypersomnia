#version 140
#extension GL_ARB_explicit_attrib_location : enable
precision mediump int;
precision mediump float;

layout(location = 0) in vec2 position;

out vec2 theTexcoord;

void main() 
{
	vec4 output_vert;
	output_vert.x = position.x * 2.0 - 1.0;		
	output_vert.y = position.y * 2.0 - 1.0;				
	output_vert.z = 0.0f;						
	output_vert.w = 1.0f;
	
	gl_Position = output_vert;
	theTexcoord = position;
}