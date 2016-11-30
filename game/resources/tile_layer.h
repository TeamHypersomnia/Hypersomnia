#pragma once
#include <vector>

#include "augs/graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "game/components/transform_component.h"

namespace resources {
	class tile_layer {
	public:
		struct tile {
			unsigned type_id;

			tile(const unsigned type = 0);
		};

		struct tile_type {
			assets::texture_id tile_texture;
			tile_type(assets::texture_id = assets::texture_id::BLANK);

			operator assets::texture_id() const {
				return tile_texture;
			}
		};

	private:
		std::vector<tile_type> tileset;
		
		std::vector<tile> tiles;
		vec2u size;

		void expand(const vec2u);
		void expand_to_position(const vec2u);
		void expand_to_rect(const ltrbu);

		unsigned get_tile_index_for_texture(const assets::texture_id);
	public:
		size_t get_tile_side() const;
		vec2u get_size() const;

		tile& tile_at(vec2u);
		const tile& tile_at(vec2u) const;

		const tile_type& get_tile_type(const tile&) const;

		void set_tile(const vec2u pos, const assets::texture_id);
		void set_tiles(const ltrbu, const assets::texture_id);
		void set_tiles_alternating(const ltrbu, const assets::texture_id, const assets::texture_id);

		struct tile_rectangular_filling {
			assets::texture_id fill;
			
			assets::texture_id left_border;
			assets::texture_id top_border;
			assets::texture_id right_border;
			assets::texture_id bottom_border;

			assets::texture_id lt_corner;
			assets::texture_id rt_corner;
			assets::texture_id rb_corner;
			assets::texture_id lb_corner;
		};

		void set_tiles(const ltrbu, const tile_rectangular_filling);
		
		struct visible_tiles_by_type {
			std::vector<std::vector<vec2u>> tiles_by_type;
		};

		visible_tiles_by_type get_visible_tiles_by_type(const ltrbu) const;
	};
}