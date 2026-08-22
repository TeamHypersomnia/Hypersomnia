precision mediump int;
precision mediump float;

in vec2 theTexcoord;
out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float bloom_intensity;

/*
	Additively composites the fully blurred bloom buffer over the scene.
	The blur itself happens earlier, in bloom_blur.fsh and bloom_blur_v.fsh.
*/

void main()
{
	vec3 glow = texture(basic_texture, theTexcoord).rgb;

	outputColor = vec4(glow * bloom_intensity, 1.0);
}
