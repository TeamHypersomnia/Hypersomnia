precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

uniform vec4 ambient_color;

/*
	Neons brighten the light texture in a few discrete bands, like quantized lights do,
	for their glassy look - even when the lights themselves aren't quantized.

	Quantized lights step the brightening by the whole light, so even faint neons cross the bands
	on top of the ambient light. Here the ambient light stands in for the light below the neon:
	the neon adds what the stepped brightening of the ambient and the neon together
	gains over that of the ambient alone, divided by the smooth brightening applied later in illuminated.fsh.
	Nothing is added where the neon is zero, and faint tails stay smooth.
*/

const float neon_levels = 3.0;

float stepped_brightening(float intensity) {
	return 1.0 + floor(min(intensity, 1.0) * neon_levels) / neon_levels;
}

void main() 
{
	vec4 pixel = theColor * texture(basic_texture, theTexcoord);

	float neon = max(max(pixel.r, pixel.g), pixel.b) * pixel.a;
	float ambient = max(max(ambient_color.r, ambient_color.g), ambient_color.b);
	float total = ambient + neon;

	float lit_with_neon = total * stepped_brightening(total) / (1.0 + min(total, 1.0));
	float lit_without = ambient * stepped_brightening(ambient) / (1.0 + min(ambient, 1.0));
	float added = max(lit_with_neon - lit_without, 0.0);

	pixel.rgb *= added / max(neon, 0.0001);
	outputColor = pixel;
}
