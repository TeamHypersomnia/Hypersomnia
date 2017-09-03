#pragma once
#include <array>
#include <algorithm>
#include <vector>

#include "augs/ensure.h"
#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos.h"

#include "game/assets/assets_declarations.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/particles_existence_component.h"

#include "game/detail/physics/physics_scripts.h"

class interpolation_system;

enum class renderable_drawing_type {
	NORMAL,
	BORDER_HIGHLIGHTS,
	NEON_MAPS
};

class render_system {
public:
	static bool render_order_compare(
		const const_entity_handle a, 
		const const_entity_handle b
	);

	template <class Container>
	void draw_entities(
		const Container& entities,
		const cosmos& cosmos,

		const augs::drawer output,
		const game_images_in_atlas& manager,

		const camera_cone in_camera,

		const double global_time_seconds,
		const interpolation_system& interp,

		const renderable_drawing_type renderable_drawing_mode
	) const {
		for (const auto e_id : entities) {
			const auto e = cosmos[e_id];

			for_each_type<components::polygon, components::sprite>([&](auto T) {
				typedef decltype(T) renderable_type;

				if (e.has<renderable_type>()) {
					const auto& render = e.get<components::render>();
					const auto& renderable_transform = e.get_viewing_transform(interp, true);
					const auto& renderable = e.get<renderable_type>();

					render_system().draw_renderable(
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
		const game_images_in_atlas& manager,

		const camera_cone camera,
		const double global_time_seconds,

		const renderable_drawing_type renderable_drawing_mode
	) const {
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

	template <class InputContainer, class OutputContainer>
	void get_visible_per_layer(
		const cosmos& cosmos,
		const InputContainer& entities,
		OutputContainer& output_layers
	) const {
		if (entities.empty()) {
			return;
		}

		for (const auto it_id : entities) {
			const auto it = cosmos[it_id];
			const auto layer = it.get<components::render>().layer;
			ensure(layer < static_cast<render_layer>(output_layers.size()));
			output_layers[layer].push_back(it);
		}

		auto& car_interior_layer = output_layers[render_layer::CAR_INTERIOR];

		if (car_interior_layer.size() > 1) {
			sort_container(
				car_interior_layer, 
				[&cosmos](const auto b, const auto a) {
					return are_connected_by_friction(cosmos[a], cosmos[b]);
				}
			);
		}
	}
};