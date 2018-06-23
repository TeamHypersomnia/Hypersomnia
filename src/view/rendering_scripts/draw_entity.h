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
#include "game/detail/frame_calculation.h"

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
	invariants::torso,
	invariants::gun,
   	invariants::polygon,
	invariants::animation
>;

template <class E, class T>
FORCE_INLINE void detail_specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	T render_visitor,
	const transformr viewing_transform
) {
	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)viewing_transform;
	(void)in;

	if constexpr(typed_handle.template has<invariants::sprite>()) {
		auto input = [&]() {
			using input_type = invariants::sprite::drawing_input;

			auto result = input_type(in.drawer);

			if (const auto flips = typed_handle.calculate_flip_flags()) { 
				result.flip = *flips;
			}

			result.renderable_transform = viewing_transform;
			result.global_time_seconds = in.global_time_seconds;

			if constexpr(typed_handle.template has<components::sprite>()) {
				const auto& sprite_comp = typed_handle.template get<components::sprite>();
				result.global_time_seconds += sprite_comp.effect_offset_secs;

				if (sprite_comp.colorize != white) {
					result.colorize *= sprite_comp.colorize;
				}
			}

			return result;
		}();

		const auto& sprite = typed_handle.template get<invariants::sprite>();

		if constexpr(typed_handle.template has<invariants::animation>()) {
			const auto& animation_def = typed_handle.template get<invariants::animation>();
			const auto& animation = typed_handle.template get<components::animation>();

			const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

			if (const auto displayed_frame = ::find_frame(animation_def, animation, logicals)) {
				const auto image_id = displayed_frame->image_id;

				auto animated = sprite;
				animated.image_id = image_id;
				animated.size = in.manager.at(image_id).get_original_size();

				render_visitor(animated, in.manager, input);
				return;
			}
		}

		if constexpr(typed_handle.template has<components::trace>()) {
			const auto trace = typed_handle.template get<components::trace>();
			const auto tracified_size = vec2(sprite.size) * trace.last_size_mult;

			if (const auto center_offset = tracified_size * trace.last_center_offset_mult;
				center_offset.non_zero()
			) {
				const auto final_rotation = input.renderable_transform.rotation;
				input.renderable_transform.pos -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
			}

			auto tracified_sprite = sprite;
			tracified_sprite.size = tracified_size;

			render_visitor(tracified_sprite, in.manager, input);
			return;
		}

		if constexpr(typed_handle.template has<components::gun>()) {
			const auto& gun = typed_handle.template get<components::gun>();
			const auto& gun_def = typed_handle.template get<invariants::gun>();

			const auto& cosm = typed_handle.get_cosmos();
			const auto& logicals = cosm.get_logical_assets();

			if (const auto shoot_animation = logicals.find(gun_def.shoot_animation)) {
				if (const auto* const frame = ::find_frame(gun, *shoot_animation, cosm)) {
					auto animated = sprite;
					animated.image_id = frame->image_id;
					animated.size = in.manager.at(frame->image_id).get_original_size();

					render_visitor(animated, in.manager, input);
					return;
				}
			}
		}

		render_visitor(sprite, in.manager, input);
		return;
	}

	if constexpr(typed_handle.template has<invariants::polygon>()) {
		using input_type = invariants::polygon::drawing_input;

		auto input = input_type(in.drawer);

		input.renderable_transform = viewing_transform;
		input.global_time_seconds = in.global_time_seconds;

		const auto& polygon = typed_handle.template get<invariants::polygon>();
		render_visitor(*polygon, in.manager, input);
		return;
	}
}

template <class E, class T>
FORCE_INLINE void specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input in,
	const interpolation_system& interp,
	T render_visitor
) {
	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)interp;
	(void)in;

	const auto viewing_transform = typed_handle.get_viewing_transform(interp);

	if (typed_handle.template has<invariants::item>()) {
		if (typed_handle.get_owning_transfer_capability().alive()) {
			/* Will be drawn when the capability itself is drawn. */
			return;
		}
	}

	if (const auto maybe_torso = typed_handle.template find<invariants::torso>()) {
		if (const auto movement = typed_handle.template find<components::movement>()) {
			const auto& cosm = typed_handle.get_cosmos();
			const auto& logicals = cosm.get_logical_assets();

			const auto velocity = typed_handle.get_effective_velocity();

			const auto legs_degrees = velocity.degrees();
			const auto face_degrees = viewing_transform.rotation;

			auto render_frame = [&in, &render_visitor, &viewing_transform](
				const auto& frame,
				const real32 rotation
			) {
				invariants::sprite sprite;
				sprite.set(frame.frame.image_id, in.manager);

				using input_type = invariants::sprite::drawing_input;

				auto input = input_type(in.drawer);
				input.renderable_transform = viewing_transform;
				input.renderable_transform.rotation = rotation;

				input.flip.vertically = frame.flip;
				render_visitor(sprite, in.manager, input);
			};

			{
				auto do_movement_animation = [&](
					const auto* const animation,
					const real32 rotation
				) {
					if (animation != nullptr) {
						render_frame(
							::get_frame_and_flip(movement->four_ways_animation, *animation),
							rotation
						);
					}
				};

				const auto leg_animation_id = maybe_torso->calc_leg_anim(velocity, face_degrees);

				do_movement_animation(logicals.find(leg_animation_id), legs_degrees);
			}

			const auto wielded_items = typed_handle.get_wielded_items();
			const auto& stance = maybe_torso->calc_stance(cosm, wielded_items);

			if (const auto stance_info = calc_stance_info(
				cosm,
				stance, 
				movement->four_ways_animation,
				wielded_items
			)) {
				render_frame(stance_info.get_with_flip(), face_degrees);

				const auto stance_image_id = stance_info.frame->image_id;
				auto stance_offsets = logicals.get_offsets(stance_image_id).torso;

				if (stance_info.flip) {
					stance_offsets.flip_vertically();
				}

				typed_handle.for_each_attachment_recursive(
					[&](
						const auto attachment_entity,
						const auto attachment_offset
					) {
						attachment_entity.dispatch(
							[&](const auto typed_attachment_handle) {
								detail_specific_entity_drawer(
									typed_attachment_handle,
									in,
									render_visitor,
									viewing_transform * attachment_offset
								);
							}
						);
					},
					[stance_offsets]() {
						return stance_offsets;
					},
					attachment_offset_settings::for_rendering()
				);

				if constexpr(typed_handle.template has<components::head>()) {
					const auto& head = typed_handle.template get<components::head>();
					const auto& head_def = typed_handle.template get<invariants::head>();

					const auto target_image = stance_info.is_shooting ? head_def.shooting_head_image : head_def.head_image;

					const auto& head_offsets = logicals.get_offsets(target_image);
					const auto target_offset = ::get_anchored_offset(stance_offsets.head, head_offsets.item.head_anchor);
					const auto target_transform = viewing_transform * target_offset;

					invariants::sprite sprite;
					sprite.set(target_image, in.manager);

					using input_type = invariants::sprite::drawing_input;

					auto input = input_type(in.drawer);
					input.renderable_transform = target_transform;
					input.renderable_transform.rotation += head.shake_rotation_amount;

					render_visitor(sprite, in.manager, input);
				}
			}
		}

		return;
	}

	detail_specific_entity_drawer(
		typed_handle,
		in,
		render_visitor,
		viewing_transform
	);
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
	if constexpr(typed_handle.template has<components::sprite>()) {
		if (typed_handle.template get<components::sprite>().disable_neon_map) {
			return;
		}
	}

	auto neon_maker  = [&](const auto& renderable, const auto& manager, auto& input) {
		input.use_neon_map = true;

		if constexpr(typed_handle.template has<components::sprite>()) {
			const auto colorize = typed_handle.template get<components::sprite>().colorize_neon;

			if (colorize != white) {
				input.colorize *= colorize;
			}
		}

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
