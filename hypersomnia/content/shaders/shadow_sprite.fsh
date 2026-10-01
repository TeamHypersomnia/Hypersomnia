precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

/*
	Nonzero while drawing into the environment shadow texture, where a blue color marks a footprint.
*/

uniform int color_encodes_footprints;

/*
	Silhouettes of foreground sprites drawn into the environment shadow texture.
	The vertex color carries what to write, see enqueue_illuminated_rendering_jobs.hpp:
	the shadow height and strength in red and green for shadows of foreground sprites,
	the strength in alpha for shadows of sprites lying on the ground,
	the height in blue for footprints of the latter.

	Also draws translucent shadows of characters over the ground, whose color is simply the shadow's.
*/

void main() 
{
	/*
		A shadow is thrown only by the sprite's solid parts, but its footprint covers every visible pixel -
		otherwise the faint edges of foliage would receive the sprite's own shadow.
	*/

	bool footprint = color_encodes_footprints != 0 && theColor.b > 0.0;
	float alpha_threshold = footprint ? 0.02 : 0.5;

	if (texture(basic_texture, theTexcoord).a < alpha_threshold) {
		discard;
	}

	outputColor = theColor;
}
