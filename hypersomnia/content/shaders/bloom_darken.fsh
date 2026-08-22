precision mediump int;
precision mediump float;

in vec2 theTexcoord;
out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float bloom_darkening;

/*
	Camera-exposure-like effect: the brighter the nearby glow,
	the more the surrounding scene gets darkened,
	so the glow itself appears blinding by contrast.

	Samples a wide neighborhood of the fully blurred bloom buffer
	and outputs black with alpha proportional to the perceived brightness.
	Composited with standard alpha blending, before the additive glow pass.
*/

const float tap_spacing = 18.0;

void main()
{
	vec2 texel = vec2(1.0) / vec2(textureSize(basic_texture, 0));

	float sum = 0.0;
	float total_weight = 0.0;

	for (int x = -3; x <= 3; ++x) {
		for (int y = -3; y <= 3; ++y) {
			vec2 off = vec2(float(x), float(y)) * tap_spacing * texel;
			vec3 s = texture(basic_texture, theTexcoord + off).rgb;

			float w = exp(-float(x * x + y * y) / 8.0);

			sum += max(s.r, max(s.g, s.b)) * w;
			total_weight += w;
		}
	}

	float brightness = sum / total_weight;
	float darkening = bloom_darkening * smoothstep(0.0, 0.3, brightness);

	outputColor = vec4(0.0, 0.0, 0.0, darkening);
}
