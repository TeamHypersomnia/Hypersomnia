precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

uniform vec4 ambient_color;

/*
	On maps that don't posterize the light as a whole, neons still brighten it in a few discrete bands
	for their glassy look.

	Posterized light steps the brightening by the whole light, so even faint neons cross the bands
	on top of the ambient light. Here the ambient light stands in for the light below the neon:
	the neon adds as much as the ambient and the neon together would get from the stepped brightening,
	relative to the brightening of the ambient alone - which the rest of the scene doesn't get.
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

	float added = max(total * stepped_brightening(total) / stepped_brightening(ambient) - ambient, 0.0);

	pixel.rgb *= added / max(neon, 0.0001);
	outputColor = pixel;
}
