#pragma once
#include "augs/math/vec2.h"
#include "game/assets/tile_layer_id.h"
#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/detail/basic_renderable_drawing_input.h"

namespace augs {
	struct texture_atlas_entry;
}

namespace components {
	struct tile_layer_instance {
		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			float global_time_seconds = 0.f;
			void set_global_time_seconds(const float);
		};

		assets::tile_layer_id id;

		tile_layer_instance(const assets::tile_layer_id = assets::tile_layer_id::INVALID);

		void draw(const drawing_input&) const;
		ltrb get_aabb(components::transform transform) const;
		ltrbu get_visible_tiles(const drawing_input&) const;
	};
};
