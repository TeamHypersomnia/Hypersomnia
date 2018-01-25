#pragma once
#include "augs/ensure.h"
#include "augs/drawing/drawing.h"
#include "augs/build_settings/platform_defines.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/render_component.h"

#include "game/detail/physics/physics_scripts.h"

#include "view/viewables/all_viewables_declarations.h"

class interpolation_system;

struct draw_renderable_input {
	const augs::drawer drawer;
	const game_images_in_atlas_map& manager;
	const double global_time_seconds;
};

template <class renderable_type>
FORCE_INLINE void draw_renderable(
	const renderable_type& renderable,
	const draw_renderable_input input,
	const components::transform renderable_transform
) {
	using input_type = typename renderable_type::drawing_input;

	auto in = input_type(input.drawer);

	in.renderable_transform = renderable_transform;
	in.set_global_time_seconds(input.global_time_seconds);

	renderable.draw(input.manager, in);
}

template <class renderable_type>
FORCE_INLINE void draw_neon_map(
	const renderable_type& renderable,
	const draw_renderable_input input,
	const components::transform renderable_transform
) {
	using input_type = typename renderable_type::drawing_input;

	auto in = input_type(input.drawer);

	in.use_neon_map = true;
	in.renderable_transform = renderable_transform;
	in.set_global_time_seconds(input.global_time_seconds);

	renderable.draw(input.manager, in);
}

template <class renderable_type>
FORCE_INLINE void draw_border(
	const renderable_type& renderable,
	const draw_renderable_input input,
	const components::transform renderable_transform,
	const rgba border_color
) {
	using input_type = typename renderable_type::drawing_input;

	auto in = input_type(input.drawer);

	in.renderable_transform = renderable_transform;
	in.set_global_time_seconds(input.global_time_seconds);

	static const vec2i offsets[4] = {
		vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
	};
	
	in.colorize = border_color;

	for (const auto o : offsets) {
		in.renderable_transform.pos = renderable_transform.pos + o;
		renderable.draw(input.manager, in);
	}
}

template <class F>
FORCE_INLINE void on_renderable_component(const const_entity_handle h, F callback) {
	if (const auto sprite = h.find_def<invariants::sprite>()) {
		if (const auto trace = h.find<components::trace>()) {
			auto tracified_sprite = *sprite;

			tracified_sprite.center_offset = tracified_sprite.size * trace->last_center_offset_mult;
			tracified_sprite.size *= trace->last_size_mult;

			callback(tracified_sprite);
		}
		else {
			callback(*sprite);
		}
	}
	else if (const auto polygon = h.find_def<invariants::polygon>()) {
		callback(*polygon);
	}
}

FORCE_INLINE void draw_entity(
	const const_entity_handle e,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	on_renderable_component(e, [&](const auto& r) {
		draw_renderable(r, in, e.get_viewing_transform(interp, true));
	});
}

FORCE_INLINE void draw_color_highlight(
	const const_entity_handle h,
	const rgba color,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	on_renderable_component(h, [&](auto copy) {
		copy.set_color(color);

		draw_renderable(
			copy,
			in,
			h.get_viewing_transform(interp, true)
		);
	});
}

FORCE_INLINE void draw_neon_map(
	const const_entity_handle e,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	on_renderable_component(e, [&](const auto& r) {
		draw_neon_map(r, in, e.get_viewing_transform(interp, true));
	});
}

template <class border_provider>
FORCE_INLINE void draw_border(
	const const_entity_handle e,
	const draw_renderable_input in,
	const interpolation_system& interp,
	const border_provider& borders
) {
	const auto border_info = borders(e);

	if (border_info) {
		on_renderable_component(e, [&](const auto& r) {
			draw_border(r, in, e.get_viewing_transform(interp, true), *border_info);
		});
	}
}