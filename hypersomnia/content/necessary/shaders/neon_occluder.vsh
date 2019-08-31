#version 130
//#extension GL_ARB_explicit_attrib_location : enable

precision mediump int;
precision mediump float;

uniform mat4 projection_matrix;
in vec2 position;
in vec2 texcoord;

out vec2 theTexcoord;

void main() 
{
	vec4 output_vert;
	output_vert.x = position.x;		
	output_vert.y = position.y;				
	output_vert.z = 0.0f;						
	output_vert.w = 1.0f;
	
	gl_Position = projection_matrix*output_vert;
	theTexcoord = texcoord;
}