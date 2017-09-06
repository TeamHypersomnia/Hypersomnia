#pragma once
#include "augs/ensure.h"
#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"
#include "augs/build_settings/platform_defines.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/render_component.h"

#include "game/detail/physics/physics_scripts.h"

#include "view/viewables/all_viewables_declarations.h"

class interpolation_system;

enum class renderable_drawing_type {
	NORMAL,
	BORDER_HIGHLIGHTS,
	NEON_MAPS
};

FORCE_INLINE bool render_order_compare(
	const const_entity_handle a, 
	const const_entity_handle b
) {
	const auto layer_a = a.get<components::render>().layer;
	const auto layer_b = b.get<components::render>().layer;

	return (layer_a == layer_b && layer_a == render_layer::CAR_INTERIOR) ? are_connected_by_friction(a, b) : layer_a < layer_b;
}

template <class Container>
void draw_entities(
	const Container& entities,
	const cosmos& cosmos,

	const augs::drawer output,
	const game_images_in_atlas_map& manager,

	const camera_cone in_camera,

	const double global_time_seconds,
	const interpolation_system& interp,

	const renderable_drawing_type renderable_drawing_mode
) {
	for (const auto e_id : entities) {
		const auto e = cosmos[e_id];

		for_each_type<components::polygon, components::sprite>([&](auto T) {
			using renderable_type = decltype(T);

			if (e.has<renderable_type>()) {
				const auto& render = e.get<components::render>();
				const auto& renderable_transform = e.get_viewing_transform(interp, true);
				const auto& renderable = e.get<renderable_type>();

				draw_renderable(
					renderable,
					renderable_transform,
					render,

					output,
					manager,

					in_camera,
					global_time_seconds,
					renderable_drawing_mode
				);
			}
		});
	}
}

template <class renderable_type>
void draw_renderable(
	const renderable_type& renderable,
	const components::transform& renderable_transform,
	const components::render& render,

	const augs::drawer output,
	const game_images_in_atlas_map& manager,

	const camera_cone camera,
	const double global_time_seconds,

	const renderable_drawing_type renderable_drawing_mode
) {
	using input_type = typename renderable_type::drawing_input;
	
	auto in = input_type(output);

	in.camera = camera;

	in.renderable_transform = renderable_transform;
	in.set_global_time_seconds(global_time_seconds);
	in.use_neon_map = renderable_drawing_mode == renderable_drawing_type::NEON_MAPS;

	if (renderable_drawing_mode == renderable_drawing_type::BORDER_HIGHLIGHTS) {
		if (render.draw_border) {
			static const vec2i offsets[4] = {
				vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
			};

			in.colorize = render.border_color;

			for (const auto o : offsets) {
				in.renderable_transform.pos = renderable_transform.pos + o;
				renderable.draw(manager, in);
			}
		}
	}
	else {
		renderable.draw(manager, in);
	}
}