precision mediump int;
precision mediump float;

in vec2 theTexcoord;
out vec4 outputColor;

uniform sampler2D basic_texture;

/*
	Vertical pass of a separable gaussian blur.
	Reads the horizontally blurred buffer (bloom_blur.fsh)
	and completes the blur of the bloom sources.
*/

const float weights[7] = float[7](0.1997, 0.1762, 0.1211, 0.0648, 0.0270, 0.0088, 0.0022);
const float tap_spacing = 3.0;

void main()
{
	vec2 texel = vec2(1.0) / vec2(textureSize(basic_texture, 0));

	vec3 sum = texture(basic_texture, theTexcoord).rgb * weights[0];

	for (int i = 1; i <= 6; ++i) {
		vec2 off = vec2(0.0, float(i) * tap_spacing * texel.y);

		sum += texture(basic_texture, theTexcoord + off).rgb * weights[i];
		sum += texture(basic_texture, theTexcoord - off).rgb * weights[i];
	}

	outputColor = vec4(sum, 1.0);
}
