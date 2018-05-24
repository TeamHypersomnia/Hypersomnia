#pragma once
#include "augs/ensure.h"
#include "augs/drawing/drawing.h"
#include "augs/build_settings/platform_defines.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/render_component.h"

#include "game/detail/physics/physics_scripts.h"

#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/images_in_atlas_map.h"

class interpolation_system;

struct draw_renderable_input {
	const augs::drawer drawer;
	const images_in_atlas_map& manager;
	const double global_time_seconds;
	const flip_flags flip;
};

using entities_with_renderables = entity_types_with_any_of<
	invariants::sprite,
   	invariants::polygon
>;

template <class E, class T>
FORCE_INLINE void specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	const interpolation_system& interp,
	T render_visitor
) {
	const auto viewing_transform = typed_handle.get_viewing_transform(interp);

	if (const auto maybe_torso = typed_handle.template find<invariants::torso>()) {
		if (const auto maybe_movement = typed_handle.template find<components::movement>()) {
			const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

			const auto chosen_leg_animation_id = maybe_torso->forward_legs;

			if (const auto* chosen_animation = mapped_or_nullptr(logicals.legs_animations, chosen_leg_animation_id)) {
				const auto duration_ms = chosen_animation->frames[0].duration_milliseconds;
				const auto amount = maybe_movement->animation_amount * 1000.f;
				const auto index = static_cast<unsigned>(amount / duration_ms);

				const auto vel = typed_handle.get_effective_velocity();
				const auto speed = vel.length();

				auto i = augs::ping_pong_with_flip(index, static_cast<unsigned>(chosen_animation->frames.size()));

				if (speed < 200 && !maybe_movement->any_moving_requested()) {
					i.first = augs::interp(0u, i.first, speed / 200);
				}

				const auto frame_id = chosen_animation->get_image_id(i.first);

				invariants::sprite sprite;
				sprite.set(frame_id, in.manager);

				using input_type = invariants::sprite::drawing_input;

				auto input = input_type(in.drawer);
				input.renderable_transform = viewing_transform;
				input.renderable_transform.rotation = vel.degrees();
				input.flip.vertically = i.second;
				render_visitor(sprite, in.manager, input);
			}

			const auto chosen_animation_id = maybe_torso->bare_walk;

			if (const auto* chosen_animation = mapped_or_nullptr(logicals.torso_animations, chosen_animation_id)) {
				const auto duration_ms = chosen_animation->frames[0].duration_milliseconds;
				const auto amount = maybe_movement->animation_amount * 1000.f;
				const auto index = static_cast<unsigned>(amount / duration_ms);

				const auto frame_id = chosen_animation->get_image_id_ping_pong_with_flip(index);

				invariants::sprite sprite;
				sprite.set(frame_id.first, in.manager);

				using input_type = invariants::sprite::drawing_input;

				auto input = input_type(in.drawer);
				input.renderable_transform = viewing_transform;
				input.flip.vertically = frame_id.second;
				render_visitor(sprite, in.manager, input);
			}
		}
	}
	else if (const auto maybe_sprite = typed_handle.template find<invariants::sprite>()) {
		const auto& sprite = *maybe_sprite;
		using input_type = invariants::sprite::drawing_input;

		auto input = input_type(in.drawer);

		if (const auto flips = typed_handle.calculate_flip_flags()) { 
			input.flip = *flips;
		}

		input.renderable_transform = viewing_transform;
		input.global_time_seconds = in.global_time_seconds;

		if (const auto trace = typed_handle.template find<components::trace>()) {
			const auto tracified_size = vec2(sprite.size) * trace->last_size_mult;

			if (const auto center_offset = tracified_size * trace->last_center_offset_mult;
				center_offset.non_zero()
			) {
				const auto final_rotation = input.renderable_transform.rotation;
				input.renderable_transform.pos -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
			}

			auto tracified_sprite = sprite;
			tracified_sprite.size = tracified_size;

			render_visitor(tracified_sprite, in.manager, input);
		}
		else {
			render_visitor(sprite, in.manager, input);
		}
	}
	else if (const auto polygon = typed_handle.template find<invariants::polygon>()) {
		using input_type = invariants::polygon::drawing_input;

		auto input = input_type(in.drawer);

		input.renderable_transform = viewing_transform;
		input.global_time_seconds = in.global_time_seconds;

		render_visitor(*polygon, in.manager, input);
	}
}

template <class E>
FORCE_INLINE void specific_draw_entity(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	auto callback = [](const auto& renderable, auto&&... args) {
		renderable.draw(std::forward<decltype(args)>(args)...);
	};

	specific_entity_drawer(typed_handle, in, interp, callback);
}

template <class E, class border_provider>
FORCE_INLINE void specific_draw_border(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	const interpolation_system& interp,
	const border_provider& borders
) {
	if (const auto border_info = borders(typed_handle)) {
		auto border_maker = [border_info](const auto& renderable, const auto& manager, auto& input) {
			const auto source_pos = input.renderable_transform.pos;
			input.colorize = *border_info;

			const vec2i offsets[4] = {
				vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
			};

			for (const auto o : offsets) {
				input.renderable_transform.pos = source_pos + o;
				renderable.draw(manager, input);
			}
		};

		specific_entity_drawer(typed_handle, in, interp, border_maker);
	}
}

template <class E>
FORCE_INLINE void specific_draw_color_highlight(
	const cref_typed_entity_handle<E> typed_handle,
	const rgba color,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	auto highlight_maker = [color](auto renderable, const auto& manager, const auto& input) {
		renderable.set_color(color);
		renderable.draw(manager, input);
	};

	specific_entity_drawer(typed_handle, in, interp, highlight_maker);
}

template <class E>
FORCE_INLINE void specific_draw_neon_map(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	auto neon_maker  = [](const auto& renderable, const auto& manager, auto& input) {
		input.use_neon_map = true;
		renderable.draw(manager, input);
	};

	specific_entity_drawer(typed_handle, in, interp, neon_maker);
}

/* Dispatching helpers */

FORCE_INLINE void draw_entity(
	const const_entity_handle handle,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, &interp](const auto typed_handle) {
		specific_draw_entity(typed_handle, in, interp);
	});
}

FORCE_INLINE void draw_neon_map(
	const const_entity_handle handle,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, &interp](const auto typed_handle) {
		specific_draw_neon_map(typed_handle, in, interp);
	});
}

FORCE_INLINE void draw_color_highlight(
	const const_entity_handle handle,
	const rgba color,
	const draw_renderable_input in,
	const interpolation_system& interp
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, &interp, color](const auto typed_handle) {
		specific_draw_color_highlight(typed_handle, color, in, interp);
	});
}

template <class B>
FORCE_INLINE void draw_border(
	const const_entity_handle handle,
	const draw_renderable_input in,
	const interpolation_system& interp,
	B&& borders
) {
	handle.conditional_dispatch<entities_with_renderables>([&in, &interp, &borders](const auto typed_handle) {
		specific_draw_border(typed_handle, in, interp, std::forward<B>(borders));
	});
}
