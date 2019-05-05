#version 140
precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D afterimage_texture;

void main() 
{
	vec4 pixel = theColor * texture(afterimage_texture, theTexcoord);
	outputColor = pixel;
}