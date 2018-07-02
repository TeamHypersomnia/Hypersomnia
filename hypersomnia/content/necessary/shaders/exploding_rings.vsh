#version 300 es
precision mediump int;
precision mediump float;


uniform mat4 projection_matrix;
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 special;

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