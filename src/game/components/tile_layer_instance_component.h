#pragma once
#include "augs/math/vec2.h"
#include "game/assets/tile_layer_id.h"
#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "game/detail/basic_renderable_drawing_input.h"

#include "game/assets/assets_manager.h"

struct all_logical_metas_of_assets;

namespace augs {
	struct texture_atlas_entry;
}

namespace components {
	struct tile_layer_instance {
		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			double global_time_seconds = 0.0;
			void set_global_time_seconds(const double);
		};

		// GEN INTROSPECTOR struct components::tile_layer_instance
		assets::tile_layer_id id;
		// END GEN INTROSPECTOR

		tile_layer_instance(const assets::tile_layer_id = assets::tile_layer_id::INVALID);

		void draw(const drawing_input) const;
		
		template <class T>
		ltrb get_aabb(
			const T& metas,
			const components::transform transform
		) const {
			const auto& layer = metas[id];
			const auto tile_square_size = layer.get_tile_side(metas);
			const auto size = layer.get_size();

			return xywh(
				transform.pos.x, 
				transform.pos.y, 
				static_cast<float>(size.x*tile_square_size), 
				static_cast<float>(size.y*tile_square_size)
			);
		}

		ltrbu get_visible_tiles(const drawing_input) const;
	};
};
