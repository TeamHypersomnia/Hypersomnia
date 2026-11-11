//#extension GL_ARB_explicit_attrib_location : enable
precision mediump int;
precision mediump float;


uniform mat4 projection_matrix;
in vec2 position;
in vec2 texcoord;
in vec4 color;

smooth out vec4 theColor;
out vec2 theTexcoord;

uniform int star_field_mode;
uniform highp vec4 star_field_transform;
uniform highp vec4 star_field_area;
uniform highp vec4 star_field_cells;
uniform highp vec4 star_field_density;
uniform highp vec4 star_field_look;
uniform vec4 star_field_color;
uniform highp int star_field_seed;
uniform highp vec4 star_field_types[48];

highp uint star_field_hash(highp uint x) {
	x ^= x >> 16u;
	x *= 0x7feb352du;
	x ^= x >> 15u;
	x *= 0x846ca68bu;
	return x ^ (x >> 16u);
}

highp float star_field_unit(highp uint h) {
	return float(h >> 8u) / 16777216.0;
}

void star_field_skip_vertex() {
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
	theColor = vec4(0.0);
	theTexcoord = vec2(0.0);
}

void star_field_vertex() {
	highp int star = gl_VertexID / 6;
	highp int vertex_in_quad = gl_VertexID - star * 6;

	highp int corner = 3;

	if (vertex_in_quad == 0 || vertex_in_quad == 3) {
		corner = 0;
	}
	else if (vertex_in_quad == 4) {
		corner = 1;
	}
	else if (vertex_in_quad == 1 || vertex_in_quad == 5) {
		corner = 2;
	}

	highp int slots_per_cell = int(star_field_cells.w);
	highp int cells_per_row = int(star_field_cells.z);
	highp int cell_index = star / slots_per_cell;
	highp int slot = star - cell_index * slots_per_cell;
	highp int cell_x = int(star_field_cells.x) + cell_index - (cell_index / cells_per_row) * cells_per_row;
	highp int cell_y = int(star_field_cells.y) + cell_index / cells_per_row;

	highp uint cell_hash = star_field_hash(uint(star_field_seed) ^ star_field_hash(uint(cell_x) ^ star_field_hash(uint(cell_y))));
	highp int stars_in_cell = int(star_field_density.x) + (star_field_unit(cell_hash) < star_field_density.y ? 1 : 0);

	if (slot >= stars_in_cell) {
		star_field_skip_vertex();
		return;
	}

	highp uint star_hash = star_field_hash(cell_hash + uint(slot) * 0x9e3779b9u);
	highp uint second_hash = star_field_hash(star_hash);
	highp uint type_hash = star_field_hash(second_hash);
	highp uint scale_hash = star_field_hash(type_hash);
	highp uint brightness_hash = star_field_hash(scale_hash);

	highp float cell_size = star_field_density.z;
	highp vec2 local_pos = (vec2(float(cell_x), float(cell_y)) + vec2(star_field_unit(star_hash), star_field_unit(second_hash))) * cell_size;

	if (local_pos.x < star_field_area.x || local_pos.x > star_field_area.z || local_pos.y < star_field_area.y || local_pos.y > star_field_area.w) {
		star_field_skip_vertex();
		return;
	}

	highp int type_count = int(star_field_density.w);
	highp float picked_weight = star_field_unit(type_hash) * star_field_look.w;
	highp int type = type_count - 1;

	for (highp int i = 0; i < type_count; ++i) {
		if (picked_weight < star_field_types[i * 3 + 2].z) {
			type = i;
			break;
		}
	}

	highp float scale = mix(star_field_look.x, star_field_look.y, star_field_unit(scale_hash));
	highp float brightness = mix(star_field_look.z, 1.0, star_field_unit(brightness_hash));

	highp vec2 size = star_field_types[type * 3 + 2].xy;
	highp vec2 corner_offset = vec2(corner == 1 || corner == 2 ? 0.5 : -0.5, corner >= 2 ? 0.5 : -0.5) * size * scale;
	highp vec2 rotated = local_pos + corner_offset;
	highp vec2 world_pos = star_field_transform.xy + vec2(
		rotated.x * star_field_transform.z - rotated.y * star_field_transform.w,
		rotated.x * star_field_transform.w + rotated.y * star_field_transform.z
	);

	highp vec4 uvs = corner < 2 ? star_field_types[type * 3] : star_field_types[type * 3 + 1];

	gl_Position = projection_matrix * vec4(world_pos, 0.0, 1.0);
	theColor = vec4(star_field_color.rgb, star_field_color.a * brightness);
	theTexcoord = corner == 0 || corner == 2 ? uvs.xy : uvs.zw;
}

void main()
{
	if (star_field_mode != 0) {
		star_field_vertex();
		return;
	}

	vec4 output_vert;
	output_vert.x = position.x;
	output_vert.y = position.y;
	output_vert.z = 0.0f;
	output_vert.w = 1.0f;

	gl_Position = projection_matrix*output_vert;
	theColor = color;
	theTexcoord = texcoord;
}
