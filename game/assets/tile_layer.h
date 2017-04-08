#pragma once
#include <vector>

#include "augs/graphics/vertex.h"
#include "game/assets/game_image_id.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"

struct tile_layer_logical_meta {
	float tile_side = 0.f;
	vec2u size;

	float get_tile_side() const {
		return tile_side;
	}

	vec2u get_size() const {
		return size;
	}
};

class tile_layer {
public:
	struct tile {
		unsigned type_id;

		tile(const unsigned type = 0);
	};

	//struct tile_type {
	//	assets::game_image_id tile_texture;
	//	unsigned max_specular_blinks = 0;
	//
	//	tile_type(assets::game_image_id = assets::game_image_id::BLANK);
	//
	//	operator assets::game_image_id() const {
	//		return tile_texture;
	//	}
	//};

	typedef unsigned tile_type_id;
	typedef components::sprite tile_type;

	// GEN INTROSPECTOR class tile_layer
	std::vector<tile_type> tileset;

	std::vector<tile> tiles;
	vec2u size;
	// END GEN INTROSPECTOR

	void expand(const vec2u);
	void expand_to_position(const vec2u);
	void expand_to_rect(const ltrbu);

public:

	float get_tile_side() const;
	vec2u get_size() const;

	tile& tile_at(vec2u);
	const tile& tile_at(vec2u) const;

	tile_type_id register_tile_type(const tile_type);

	tile_type& get_tile_type(const tile_type_id);
	const components::sprite& get_tile_type(const tile_type_id) const;

	tile_type& get_tile_type(const tile&);
	const tile_type& get_tile_type(const tile&) const;

	void set_tile(const vec2u pos, const tile_type_id);
	void set_tiles(const ltrbu, const tile_type_id);
	void set_tiles_alternating(const ltrbu, const tile_type_id, const tile_type_id);

	struct tile_rectangular_filling {
		tile_type_id fill;

		tile_type_id left_border;
		tile_type_id top_border;
		tile_type_id right_border;
		tile_type_id bottom_border;

		tile_type_id lt_corner;
		tile_type_id rt_corner;
		tile_type_id rb_corner;
		tile_type_id lb_corner;
	};

	void set_tiles(const ltrbu, const tile_rectangular_filling);

	struct visible_tiles_by_type {
		std::vector<std::vector<vec2u>> tiles_by_type;
	};

	visible_tiles_by_type get_visible_tiles_by_type(const ltrbu) const;

	tile_layer_logical_meta get_logical_meta() const;
};
