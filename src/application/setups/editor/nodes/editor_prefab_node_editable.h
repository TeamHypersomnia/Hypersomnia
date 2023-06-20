#pragma once

struct editor_sprite_resource;

struct aquarium_prefab_node {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct aquarium_prefab_node

	editor_typed_resource_id<editor_light_resource> point_light;
	editor_typed_resource_id<editor_area_marker_resource> organism_area;

	editor_typed_resource_id<editor_sprite_resource> sand_1;
	editor_typed_resource_id<editor_sprite_resource> sand_2;
	editor_typed_resource_id<editor_sprite_resource> sand_edge;

	editor_typed_resource_id<editor_sprite_resource> dune_small;
	editor_typed_resource_id<editor_sprite_resource> dune_big;

	editor_typed_resource_id<editor_sprite_resource> wall;
	editor_typed_resource_id<editor_sprite_resource> wall_top_corners;
	editor_typed_resource_id<editor_sprite_resource> wall_bottom_corners;
	editor_typed_resource_id<editor_sprite_resource> wall_smooth_end;

	editor_typed_resource_id<editor_sprite_resource> wall_top_foreground;

	editor_typed_resource_id<editor_sprite_resource> glass;
	editor_typed_resource_id<editor_sprite_resource> glass_start;

	editor_typed_resource_id<editor_sprite_resource> sand_lamp_body;
	editor_typed_resource_id<editor_sprite_resource> sand_lamp_light;

	editor_typed_resource_id<editor_sprite_resource> wall_lamp_body;
	editor_typed_resource_id<editor_sprite_resource> wall_lamp_light;

	editor_typed_resource_id<editor_sprite_resource> flower_1;
	editor_typed_resource_id<editor_sprite_resource> flower_2;
	editor_typed_resource_id<editor_sprite_resource> coral;

	editor_typed_resource_id<editor_sprite_resource> fish_1;
	editor_typed_resource_id<editor_sprite_resource> fish_2;
	editor_typed_resource_id<editor_sprite_resource> fish_3;
	editor_typed_resource_id<editor_sprite_resource> fish_4;
	editor_typed_resource_id<editor_sprite_resource> fish_5;
	editor_typed_resource_id<editor_sprite_resource> fish_6;

	editor_typed_resource_id<editor_particles_resource> bubbles;
	editor_typed_resource_id<editor_particles_resource> flower_bubbles;

	editor_typed_resource_id<editor_wandering_pixels_resource> wandering_pixels_1;
	editor_typed_resource_id<editor_wandering_pixels_resource> wandering_pixels_2;

	editor_typed_resource_id<editor_sound_resource> ambience_left;
	editor_typed_resource_id<editor_sound_resource> ambience_right;

	editor_typed_resource_id<editor_sprite_resource> water_overlay;
	editor_typed_resource_id<editor_sprite_resource> collider_interior;

	editor_typed_resource_id<editor_sprite_resource> caustics;

	bool flip_glass_vertically = false;
	int glass_start_offset = 10;
	int wall_lamp_offset = 50;

	uint32_t fish_seed = 1337;

	uint32_t fish_1_count = 12;
	uint32_t fish_2_count = 8;
	uint32_t fish_3_count = 6;
	uint32_t fish_4_count = 6;
	uint32_t fish_5_count = 4;
	uint32_t fish_6_count = 4;

	uint32_t caustics_seed = 8933;
	uint32_t caustics_count = 9;

	uint32_t dim_caustics_seed = 8204;
	uint32_t dim_caustics_count = 4;

	rgba left_bottom_lamp_color = rgba(96, 255, 255, 255);
	rgba right_top_lamp_color = rgba(103, 255, 69, 255);
	rgba top_lamp_color = rgba(0, 179, 255, 0);
	rgba sand_lamp_color = rgba(0, 99, 126, 255);
	// END GEN INTROSPECTOR

	bool operator==(const aquarium_prefab_node&) const = default;
};

struct editor_prefab_node_editable {
	// GEN INTROSPECTOR struct editor_prefab_node_editable
	vec2 pos;
	real32 rotation = 0.0f;
	vec2i size = { 256, 256 };

	aquarium_prefab_node as_aquarium;
	// END GEN INTROSPECTOR

	bool operator==(const editor_prefab_node_editable&) const = default;
};
