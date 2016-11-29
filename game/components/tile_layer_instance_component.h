#pragma once
#include "augs/math/vec2.h"
#include "game/assets/tile_layer_id.h"
#include "transform_component.h"
#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/detail/camera_cone.h"

namespace augs {
	class texture;
}

namespace components {
	struct tile_layer_instance {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;
			components::transform renderable_transform;
			camera_cone camera;
			augs::rgba colorize;
			bool use_neon_map = false;
		};

		assets::tile_layer_id id;

		tile_layer_instance(const assets::tile_layer_id = assets::tile_layer_id::INVALID);

		void draw(const drawing_input&) const;
		ltrb get_aabb(components::transform transform) const;
		ltrbu get_visible_tiles(const drawing_input&) const;
	};
};
