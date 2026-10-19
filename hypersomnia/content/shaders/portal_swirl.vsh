precision highp int;
precision highp float;

uniform mat4 projection_matrix;
in vec2 position;
in vec2 texcoord;
in vec4 color;
in vec4 special;

smooth out vec4 theColor;
flat out vec4 portal;

void main() {
	gl_Position = projection_matrix * vec4(position.xy, 0.0, 1.0);
	theColor = color;

	/* Center on the screen (GL coordinates), radius in screen pixels with a seed, the grain's color - see portal_swirl.fsh. */
	portal = special;
}
