#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_sprite_resource;

struct editor_sprite_node_editable {
	// GEN INTROSPECTOR struct editor_sprite_node_editable
	vec2 pos = vec2::zero;
	real32 rotation = 0.0f;
	augs::maybe<vec2i, false> size;
	rgba colorize = white;

	bool flip_horizontally = false;
	bool flip_vertically = false;
	augs::maybe<uint32_t> starting_animation_frame = augs::maybe<uint32_t>(0, false);
	bool randomize_starting_animation_frame = false;
	bool randomize_color_wave_offset = false;

	float animation_speed_factor = 1.0f;
	// END GEN INTROSPECTOR
};

struct editor_sprite_node : editor_node_base<
	editor_sprite_resource,
	editor_sprite_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Sprite";
	}
};
