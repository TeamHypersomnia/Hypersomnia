#pragma once
#include <algorithm>
#include <cmath>
#include "augs/ensure.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/sprite.hpp"
#include "augs/build_settings/compiler_defines.h"

#include "game/enums/render_layer.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/destructible_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/render_component.h"
#include "game/components/remnant_component.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/frame_calculation.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/audiovisual_state/systems/randomizing_system.h"
#include "view/rendering_scripts/draw_entity_input.h"
#include "game/components/torso_component.hpp"

using entities_with_renderables = entity_types_having_any_of<
	invariants::sprite,
	invariants::torso,
	invariants::gun,
   	invariants::polygon,
	invariants::animation
>;

enum class head_drawing_type {
	NONE,
	DIM,
	NORMAL
};

inline auto calc_head_drawing_type(const components::sentience& sentience) {
	if (!sentience.is_dead()) {
		return head_drawing_type::NORMAL;
	}

	return sentience.detached.head.is_set() ? head_drawing_type::NONE : head_drawing_type::DIM;
}

template <bool for_gui = false, class E, class T>
FORCE_INLINE void detail_specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const specific_draw_input& in,
	T render_visitor,
	const transformr viewing_transform,
	const bool flip_vertically = false,
	const float mult_alpha = 1.0f
) {
	using H = cref_typed_entity_handle<E>;

	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)viewing_transform;
	(void)in;

	if constexpr(H::template has<invariants::sprite>()) {
		float teleport_alpha = 1.0f;

		if constexpr(!for_gui && H::template has<components::rigid_body>()) {
			teleport_alpha = typed_handle.template get<components::rigid_body>().get_teleport_alpha();
		}

		teleport_alpha *= mult_alpha;

		const auto sprite = [&typed_handle]() {
			auto result = typed_handle.template get<invariants::sprite>();

			if constexpr(H::template has<components::overridden_geo>()) {
				const auto geo = typed_handle.template get<components::overridden_geo>();
				const auto& s = geo.get();

				if (s.is_enabled) {
					result.size = s.value;
				}
			}

			/* 
			 * If the entity has a destructible component with non-default texture_rect,
			 * scale the sprite size accordingly. The texture_rect represents the portion
			 * of the original texture that is still visible after destruction.
			 */
			if constexpr(H::template has<components::destructible>()) {
				const auto& destructible = typed_handle.template get<components::destructible>();
				const auto& tex_rect = destructible.texture_rect;
				/* Use float arithmetic and round to avoid precision loss */
				result.size.x = static_cast<int>(std::round(static_cast<float>(result.size.x) * tex_rect.w));
				result.size.y = static_cast<int>(std::round(static_cast<float>(result.size.y) * tex_rect.h));
				/* Ensure minimum size of 1 to prevent zero-size sprites */
				result.size.x = std::max(1, result.size.x);
				result.size.y = std::max(1, result.size.y);
			}

			return result;
		}();

		auto input = [&]() {
			auto result = in.make_input_for<invariants::sprite>();

			if (const auto flips = typed_handle.calc_flip_flags()) { 
				result.flip = *flips;
			}

			if (sprite.tile_excess_size) {
				result.tile_size = typed_handle.template get<invariants::sprite>().size;
			}

			result.renderable_transform = viewing_transform;
			result.global_time_seconds = in.global_time_seconds;

			if (flip_vertically) {
				result.flip.vertically = !result.flip.vertically;
			}

			/* Pass texture_rect and tile_offset for destructible sprites */
			if constexpr(H::template has<components::destructible>()) {
				const auto& destructible = typed_handle.template get<components::destructible>();
				result.texture_rect = destructible.texture_rect;
				
				/* 
				 * For tiled sprites, calculate the tile offset to maintain visual continuity.
				 * The offset is where this chunk starts within the original sprite's tile grid.
				 */
				if (sprite.tile_excess_size) {
					const auto original_sprite_size = typed_handle.template get<invariants::sprite>().size;
					result.tile_offset = vec2(
						original_sprite_size.x * destructible.texture_rect.x,
						original_sprite_size.y * destructible.texture_rect.y
					);
				}
			}

			{
				const auto& v = sprite.neon_alpha_vibration;

				if (v.is_enabled && sprite.vibrate_diffuse_too) {
					const auto id = typed_handle.get_id();
					const auto mult = in.randomizing.get_random_walk_mult(id, v.value);

					result.colorize.mult_alpha(mult);
				}
			}

			if constexpr(H::template has<components::sprite>()) {
				const auto& sprite_comp = typed_handle.template get<components::sprite>();
				result.global_time_seconds += sprite_comp.effect_offset_secs;

				if (sprite_comp.colorize != white) {
					result.colorize *= sprite_comp.colorize;
				}

				result.disable_special_effects = sprite_comp.disable_special_effects;
			}

			result.colorize.mult_alpha(teleport_alpha);

			return result;
		}();

		if constexpr(!for_gui) {
			if constexpr(H::template has<components::hand_fuse>()) {
				const auto& fuse = typed_handle.template get<components::hand_fuse>();
				const auto& fuse_def = typed_handle.template get<invariants::hand_fuse>();

				const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

				const auto& animation = typed_handle.template get<components::animation>();

				if (fuse.defused()) {
					if (fuse_def.defused_image_id.is_set()) {
						auto defused = sprite;
						defused.image_id = fuse_def.defused_image_id;

						render_visitor(defused, in.manager, input);
					}

					return;
				}

				if (fuse.when_armed.was_set()) {
					if (const auto displayed_frame = ::find_frame(fuse_def.armed_animation_id, animation, logicals)) {
						const auto image_id = displayed_frame->image_id;

						auto animated = sprite;
						animated.image_id = image_id;
						animated.size = in.manager.at(image_id).get_original_size();

						render_visitor(animated, in.manager, input);
						return;
					}
				}
			}

			if constexpr(H::template has<invariants::animation>()) {
				const auto& animation_def = typed_handle.template get<invariants::animation>();
				const auto& animation = typed_handle.template get<components::animation>();

				const auto& logicals = typed_handle.get_cosmos().get_logical_assets();

				if (const auto displayed_frame = ::find_first_and_current_frame(animation_def, animation, logicals);
					displayed_frame.first && displayed_frame.second
				) {
					const auto reference_image_id = displayed_frame.first->image_id;
					const auto displayed_image_id = displayed_frame.second->image_id;

					auto animated = sprite;
					animated.image_id = displayed_image_id;

					const auto this_frame_size_mult = 
						vec2(in.manager.at(displayed_image_id).get_original_size())
						   / in.manager.at(reference_image_id).get_original_size()
					;

					animated.size = vec2i(vec2(animated.size) * this_frame_size_mult);

					render_visitor(animated, in.manager, input);
					return;
				}
			}

			if constexpr(H::template has<components::trace>()) {
				const auto& trace = typed_handle.template get<components::trace>();

				if (trace.enabled) {
					const auto tracified_size = vec2(sprite.size) * trace.last_size_mult;

					if (const auto center_offset = tracified_size * trace.last_center_offset_mult;
						center_offset.is_nonzero()
					) {
						const auto final_rotation = input.renderable_transform.rotation;
						input.renderable_transform.pos -= vec2(center_offset).rotate(final_rotation);
					}

					auto tracified_sprite = sprite;
					tracified_sprite.size = tracified_size;

					render_visitor(tracified_sprite, in.manager, input);
					return;
				}
			}

			if constexpr(H::template has<components::remnant>()) {
				const auto& remnant = typed_handle.template get<components::remnant>();

				auto remnanted_sprite = sprite;
				remnanted_sprite.size *= remnant.last_size_mult;

				render_visitor(remnanted_sprite, in.manager, input);
				return;
			}

			if constexpr(H::template has<components::decal>()) {
				const auto& remnant = typed_handle.template get<components::decal>();

				auto remnanted_sprite = sprite;
				remnanted_sprite.size *= remnant.last_size_mult;

				render_visitor(remnanted_sprite, in.manager, input);
				return;
			}

			if constexpr(H::template has<components::gun>()) {
				const auto& gun = typed_handle.template get<components::gun>();
				const auto& gun_def = typed_handle.template get<invariants::gun>();

				const auto& cosm = typed_handle.get_cosmos();
				const auto& logicals = cosm.get_logical_assets();

				if (const auto shoot_animation = logicals.find(gun_def.shoot_animation)) {
					if (const auto* const frame = ::find_shoot_frame(gun_def, gun, *shoot_animation, cosm)) {
						auto animated = sprite;
						animated.image_id = frame->image_id;
						animated.size = in.manager.at(frame->image_id).get_original_size();

						render_visitor(animated, in.manager, input);
						return;
					}
				}
			}
		}

		render_visitor(sprite, in.manager, input);
		return;
	}

#if 0
	if constexpr(H::template has<invariants::polygon>()) {
		auto input = in.make_input_for<invariants::polygon>();

		input.renderable_transform = viewing_transform;
		input.global_time_seconds = in.global_time_seconds;

		const auto& polygon = typed_handle.template get<invariants::polygon>();
		render_visitor(*polygon, in.manager, input);
		return;
	}
#endif
}

template <bool is_neon = false, class E, class T>
FORCE_INLINE void specific_entity_drawer(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	T render_visitor
) {
	using H = cref_typed_entity_handle<E>;

	/* Might or might not be used depending on if constexpr flow */
	(void)render_visitor;
	(void)in;

	const auto viewing_transform = typed_handle.get_viewing_transform(in.interp);

	if constexpr (H::template has<components::item>()) {
		if (typed_handle.get_current_slot().alive()) {
			/* Will be drawn when the capability itself is drawn. */
			return;
		}
		else {
			auto draw_attachment = [&](
				const auto attachment_entity,
				const auto attachment_offset
			) {
				detail_specific_entity_drawer(
					attachment_entity,
					in,
					render_visitor,
					viewing_transform * attachment_offset.offset
				);
			};

			typed_handle.with_each_attachment_recursive(
				draw_attachment,
				attachment_offset_settings::for_rendering()
			);

			return;
		}
	}

	if constexpr (H::template has<invariants::torso>()) {
		if constexpr (H::template has<components::movement>()) {
			float teleport_alpha = typed_handle.template get<components::rigid_body>().get_teleport_alpha();

			const auto& sentience = typed_handle.template get<components::sentience>();

			const auto& torso = typed_handle.template get<invariants::torso>();
			const auto& movement = typed_handle.template get<components::movement>();

			const auto& cosm = typed_handle.get_cosmos();

			float spawn_prot_mult = 1.0f;

			if (sentience.spawn_protection_cooldown.lasts(cosm.get_clock())) {
				spawn_prot_mult = 0.45f;
			}

			teleport_alpha *= spawn_prot_mult;

			const auto& logicals = cosm.get_logical_assets();

			const auto velocity = typed_handle.get_effective_velocity();

			const auto face_degrees = viewing_transform.rotation;

			auto draw_torso_frame = [teleport_alpha, &in, &render_visitor, &typed_handle](
				const auto& frame,
				const transformr where
			) {
				invariants::sprite sprite;
				sprite.set(frame.frame.image_id, in.manager);
				const auto& original_sprite = typed_handle.template get<invariants::sprite>();
				sprite.color = original_sprite.color;

				auto input = in.make_input_for<invariants::sprite>();
				input.renderable_transform = where;
				input.colorize.mult_alpha(teleport_alpha);

				input.flip = frame.flip;
				render_visitor(sprite, in.manager, input);
			};

			const auto wielded_items = typed_handle.get_wielded_items();
			const bool consider_weapon_reloading = true;
			const auto stance_id = ::calc_stance_id(typed_handle, wielded_items, consider_weapon_reloading);
			const auto& stance = torso.stances[stance_id];

			auto four_ways = movement.four_ways_animation;

			if (movement.was_walk_effective) {
				four_ways.index = 0;
			}

			if (const auto stance_usage = calc_stance_usage(
				typed_handle,
				stance, 
				four_ways,
				wielded_items
			)) {
				const auto stance_offsets = [&stance_usage, &logicals]() {
					const auto stance_image_id = stance_usage.frame->image_id;

					auto result = logicals.get_offsets(stance_image_id).torso;

					if (stance_usage.movement_flip.vertically) {
						result.flip_vertically();
					}

					return result;
				}();

				{
					const auto leg_anim = torso.calc_leg_anim(velocity, face_degrees + stance_offsets.strafe_facing_offset);

					if (const auto animation = logicals.find(leg_anim.id);
						animation != nullptr
					) {
						const auto& legs = stance_offsets.legs;
						const auto leg_offset = transformr(legs.pos, legs.rotation);

						auto frame_with_flip = ::get_frame_and_flip(four_ways, *animation);

						if (leg_anim.flip_vertically) {
							auto& f = frame_with_flip.flip.vertically;
							f = !f;
						}
						
						draw_torso_frame(
							frame_with_flip,
							{ (viewing_transform * leg_offset).pos, leg_anim.rotation }
						);
					}
				}

				/* Draw items under the sentience first. */

				const auto reloading_movement = ::calc_reloading_movement(typed_handle.get_cosmos(), wielded_items);
				const bool currently_reloading = reloading_movement.has_value();

				auto should_draw_over_torso = [&](const auto attachment_entity) {
					const auto slot = attachment_entity.get_current_slot();

					if (slot.get_type() == slot_function::BELT) {
						return false;
					}

					if (slot.is_hand_slot()) {
						const auto& item_def = attachment_entity.template get<invariants::item>();

						if (currently_reloading) {
							return item_def.draw_over_hands_when_reloading;
						}

						if (stance_offsets.is_akimbo && item_def.draw_under_hands_in_akimbo) {
							return false;
						}

						return item_def.draw_over_hands;
					}

					return true;
				};

				auto get_offsets_by_torso = [stance_offsets]() {
					return stance_offsets;
				};

				const bool draw_mag_over = 
					currently_reloading
					? cosm[reloading_movement->weapon].template get<invariants::item>().draw_mag_over_when_reloading
					: false
				;

				auto draw_items_recursively = [&](const bool over_torso = false) {
					auto draw_attachment = [&](
						const auto attachment_entity,
						const auto attachment_offset
					) {
						attachment_entity.template dispatch_on_having_all<invariants::item>(
							[&](const auto typed_attachment_handle) {
								detail_specific_entity_drawer(
									typed_attachment_handle,
									in,
									render_visitor,
									viewing_transform * attachment_offset.offset,
									attachment_offset.flip_geometry,
									spawn_prot_mult
								);
							}
						);
					};

					auto should_recurse = [over_torso, &should_draw_over_torso](const auto& slot) {
						if (slot.is_hand_slot()) {
							if (const auto held_item = slot.get_item_if_any()) {
								return over_torso == should_draw_over_torso(held_item);
							}
						}

						return true;
					};

					typed_handle.recurse_character_attachments(
						draw_attachment,
						should_recurse,
						get_offsets_by_torso,
						attachment_offset_settings::for_rendering(),
						!draw_mag_over
					);
				};

				draw_items_recursively();

				const bool only_secondary = typed_handle.only_secondary_holds_item();

				{
					/* Draw the actual torso */
					auto usage = stance_usage;

					if (only_secondary) {
						auto& f = usage.movement_flip.vertically;
						f = !f;
					}

					draw_torso_frame(usage.get_with_flip(), { viewing_transform.pos, face_degrees });
				}

				draw_items_recursively(true);

				const auto head_drawing = calc_head_drawing_type(sentience);

				if (head_drawing != head_drawing_type::NONE) {
					if constexpr(typed_handle.template has<components::head>()) {
						const auto& head = typed_handle.template get<components::head>();
						const auto& head_def = typed_handle.template get<invariants::head>();

						const auto target_image = 
							stance_usage.flags.test(stance_flag::SHOOTING)
							? head_def.shooting_head_image 
							: head_def.head_image
						;

						const auto& head_offsets = logicals.get_offsets(target_image);

						auto stance_offsets_for_head = stance_offsets;
						auto anchor_for_head = head_offsets.item.head_anchor;

						if (wielded_items.size() == 1) {
							if (const auto w = cosm[wielded_items[0]]) {
								anchor_for_head += logicals.get_offsets(w.get_image_id()).item.head_anchor;
							} 
						}

						if (only_secondary) {
							stance_offsets_for_head.flip_vertically();
							anchor_for_head.flip_vertically();
						}

						const auto target_offset = ::get_anchored_offset(stance_offsets_for_head.head, anchor_for_head);
						const auto target_transform = viewing_transform * target_offset;

						invariants::sprite sprite;
						sprite.set(target_image, in.manager);

						auto input = in.make_input_for<invariants::sprite>();
						input.renderable_transform = target_transform;
						input.renderable_transform.rotation += head.shake_rotation_amount;

						const auto head_dim = head_drawing == head_drawing_type::DIM;
						const bool should_draw_dim_but_this_is_neon = head_dim && is_neon;

						if (head_dim) {
							input.colorize.multiply_rgb(0.75f);
						}

						input.colorize.mult_alpha(teleport_alpha);

						if (!should_draw_dim_but_this_is_neon) {
							render_visitor(sprite, in.manager, input);
						}
					}
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

struct default_customize_input {
	template <class I>
	decltype(auto) operator()(I&& input) const {
		return std::forward<I>(input);
	}
};

template <class E, class F = default_customize_input>
FORCE_INLINE void specific_draw_entity(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	F&& customize_input = default_customize_input()
) {
	auto callback = [&customize_input](const auto& renderable, const auto& manager, const auto& input) {
		augs::draw(renderable, manager, customize_input(input));
	};

	specific_entity_drawer(typed_handle, in, callback);
}

template <class E, class border_provider, class F = default_customize_input>
FORCE_INLINE void specific_draw_border(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	const border_provider& borders,
	F&& customize_input = default_customize_input()
) {
	if (const auto border_info = borders(typed_handle)) {
		auto border_maker = [border_info, customize_input](const auto& renderable, const auto& manager, auto& pre_input) {
			auto input = customize_input(pre_input);

			const auto source_pos = input.renderable_transform.pos;
			input.colorize = *border_info;

			const vec2i offsets[4] = {
				vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
			};

			for (const auto& o : offsets) {
				input.renderable_transform.pos = source_pos + o;
				augs::draw(renderable, manager, input);
			}
		};

		specific_entity_drawer(typed_handle, in, border_maker);
	}
}

template <class E, class F = default_customize_input>
FORCE_INLINE void specific_draw_color_highlight(
	const cref_typed_entity_handle<E> typed_handle,
	const rgba color,
	const draw_renderable_input& in,
	F&& customize_input = default_customize_input(),
	vec2 size_mult = vec2(1.0f, 1.0f)
) {
	auto highlight_maker = [color, &customize_input, size_mult](auto renderable, const auto& manager, const auto& input) {
		renderable.set_color(color);
		renderable.size *= size_mult;
		augs::draw(renderable, manager, customize_input(input));
	};

	specific_entity_drawer(typed_handle, in, highlight_maker);
}

template <class E, class F = default_customize_input>
FORCE_INLINE void specific_draw_neon_map(
	const cref_typed_entity_handle<E> typed_handle,
	const draw_renderable_input& in,
	F&& customize_input = default_customize_input()
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

		{
			const auto& v = renderable.neon_alpha_vibration;

			if (v.is_enabled) {
				const auto id = typed_handle.get_id();
				const auto mult = in.randomizing.get_random_walk_mult(id, v.value);

				input.colorize.mult_alpha(mult);
			}
		}

		augs::draw(renderable, manager, customize_input(input));
	};

	specific_entity_drawer<true>(typed_handle, in, neon_maker);
}

/* Dispatching helpers */

FORCE_INLINE void draw_entity(
	const const_entity_handle& handle,
	const draw_renderable_input& in
) {
	handle.constrained_dispatch<entities_with_renderables>([&in](const auto typed_handle) {
		specific_draw_entity(typed_handle, in);
	});
}

FORCE_INLINE void draw_neon_map(
	const const_entity_handle& handle,
	const draw_renderable_input& in
) {
	handle.constrained_dispatch<entities_with_renderables>([&in](const auto typed_handle) {
		specific_draw_neon_map(typed_handle, in);
	});
}

FORCE_INLINE void draw_color_highlight(
	const const_entity_handle& handle,
	const rgba color,
	const draw_renderable_input& in
) {
	handle.constrained_dispatch<entities_with_renderables>([&in, color](const auto typed_handle) {
		specific_draw_color_highlight(typed_handle, color, in);
	});
}

template <class B>
FORCE_INLINE void draw_border(
	const const_entity_handle& handle,
	const draw_renderable_input& in,
	B&& borders
) {
	handle.constrained_dispatch<entities_with_renderables>([&in, &borders](const auto typed_handle) {
		specific_draw_border(typed_handle, in, std::forward<B>(borders));
	});
}
