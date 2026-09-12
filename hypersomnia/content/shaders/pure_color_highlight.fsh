precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

/*
	When positive, texture pixels darker than the cutoff fade out.
	Lets silhouettes skip the baked-in black outlines of sprites.
	Defaults to 0.0 so all other usages are unaffected.
*/
uniform float black_cutoff;

void main()
{
	vec4 pixel = theColor;
	vec4 tex = texture(basic_texture, theTexcoord);
	pixel.a *= tex.a;

	if (black_cutoff > 0.0) {
		pixel.a *= smoothstep(0.0, black_cutoff, max(tex.r, max(tex.g, tex.b)));
	}

	outputColor = pixel;
}
