precision mediump int;
precision mediump float;

in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform vec4 global_color;

void main() 
{
	vec4 pixel = global_color;
	pixel.a *= texture(basic_texture, theTexcoord).a;
	outputColor = pixel;
}