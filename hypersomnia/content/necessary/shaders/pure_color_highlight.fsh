#version 300 es
precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main() 
{
	vec4 pixel = theColor;
	pixel.a *= texture(basic_texture, theTexcoord).a;
	outputColor = pixel;
}