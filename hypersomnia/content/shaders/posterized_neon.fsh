precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

uniform vec4 ambient_color;

/*
	Nonzero when the map brightens all light smoothly afterwards (Physical lights with Brighten lights) -
	then the banded neon is divided by the smooth brightening it will get, instead of being brightened twice.
	The division is smooth, so the steps between the bands stay as sharp as they are.
*/

uniform int smooth_brightening;

/*
	Neons brighten the light in a few discrete bands for their glassy look -
	on maps whose light isn't banded as a whole already.

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

	float added = 0.0;

	if (smooth_brightening != 0) {
		/*
			Relative to the ambient alone, which gets 1 + ambient from the smooth brightening,
			the light here gets about 1 + total.
		*/

		float banded = total * stepped_brightening(total) / stepped_brightening(ambient);
		added = max(banded * (1.0 + ambient) / (1.0 + min(total, 1.0)) - ambient, 0.0);
	}
	else {
		added = max(total * stepped_brightening(total) / stepped_brightening(ambient) - ambient, 0.0);
	}

	pixel.rgb *= added / max(neon, 0.0001);
	outputColor = pixel;
}
