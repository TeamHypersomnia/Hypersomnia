//#extension GL_ARB_explicit_attrib_location : enable
precision mediump int;
precision mediump float;


uniform mat4 projection_matrix;
in vec2 position;
in vec2 texcoord;
in vec4 color;
in vec4 special;

smooth out vec4 theColor;
out float inner_radius_sq;
out float outer_radius_sq;
out vec2 ring_center;

void main() {
	vec4 output_vert;
	output_vert.x = position.x;		
	output_vert.y = position.y;				
	output_vert.z = 0.0f;						
	output_vert.w = 1.0f;

	ring_center = special.xy;
	inner_radius_sq = special.z*special.z;
	outer_radius_sq = special.w*special.w;

	gl_Position = projection_matrix * output_vert;
	theColor = color;
}