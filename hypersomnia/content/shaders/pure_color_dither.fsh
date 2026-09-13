precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

void main()
{
	/*
		Discard every other pixel in a 2x2 checkerboard pattern.
		Used by the minimap for obstacles that block walking
		but let bullets fly through.
	*/
	if (mod(floor(gl_FragCoord.x) + floor(gl_FragCoord.y), 2.0) < 0.5) {
		discard;
	}

	outputColor = theColor;
}
