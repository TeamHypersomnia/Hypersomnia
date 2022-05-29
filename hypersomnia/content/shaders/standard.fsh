precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main() 
{
	vec4 pixel = theColor * texture(basic_texture, theTexcoord);
	outputColor = pixel;
}