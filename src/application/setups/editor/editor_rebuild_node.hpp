#pragma once
#include "application/setups/editor/to_game_effect.hpp"
#include "augs/misc/enum/enum_bitset.h"

template <class T>
void make_unselectable(T& agg) {
	agg.when_born.step = 2;
}

template <class T>
void make_unselectable_handle(T handle) {
	::make_unselectable(handle.get({}));
}

template <class G, class F, class N, class R, class H, class A>
bool setup_entity_from_node(
	G get_asset_id_of,
	F find_resource,
	const sorting_order_type total_order,
	const editor_layer& layer,
	const N& node, 
	const R& resource,
	H& handle, 
	A& agg
) {
	auto to_game_effect = [&](const auto& from) {
		return ::convert_to_game_effect(get_asset_id_of, find_resource, from);
	};

	bool dependent_on_other_nodes = false;

	using Editable = decltype(node.editable);
	auto& editable = node.editable;

	if (auto sorting_order = agg.template find<components::sorting_order>()) {
		sorting_order->order = total_order;
	}
	
	ensure(agg.when_born.step == 1);

	if (!layer.editable.selectable_on_scene) {
		::make_unselectable(agg);
	}

	if constexpr(std::is_same_v<N, editor_sprite_node>) {
		auto& sprite = agg.template get<components::sprite>();
		sprite.colorize = editable.color;
		sprite.colorize *= layer.editable.tint;

		sprite.colorize_neon = editable.neon_color;

		const auto opacity = layer.editable.opacity;

		if (opacity != 1.0f) {
			sprite.colorize.mult_alpha(opacity);
		}

		if (auto body = agg.template find<components::rigid_body>()) {
			body->special.penetrability = editable.penetrability;
		}
	}
	else if constexpr(std::is_same_v<N, editor_light_node>) {
		auto set_attn_from_falloff = [&](auto& attn, const auto& foff) {
			const auto mult = foff.calc_attenuation_mult_for_requested_radius();

			attn.constant = foff.constant * mult * CONST_MULT;
			attn.linear = foff.linear * mult * LINEAR_MULT;
			attn.quadratic = foff.quadratic * mult * QUADRATIC_MULT;

			attn.trim_alpha = foff.strength;
			// LOG_NVPS(foff.radius, attn.calc_reach(), attn.calc_reach_trimmed());
		};

		auto& light = agg.template get<components::light>();
		light.color *= editable.color;

		set_attn_from_falloff(light.attenuation, node.editable.falloff);

		{
			auto& wall_foff = node.editable.wall_falloff;

			if (wall_foff.is_enabled) {
				set_attn_from_falloff(light.wall_attenuation, wall_foff.value);
			}
			else {
				auto auto_wall_falloff = node.editable.falloff;
				auto_wall_falloff.radius /= 1.5f; 

				set_attn_from_falloff(light.wall_attenuation, auto_wall_falloff);
			}
		}

		light.wall_variation = light.variation;

		{
			const float positional_vibration = node.editable.positional_vibration;
			const float intensity_vibration = node.editable.intensity_vibration;

			light.variation.is_enabled = intensity_vibration > 0.0f;
			light.wall_variation.is_enabled = intensity_vibration > 0.0f;

			light.position_variations.is_enabled = positional_vibration > 0.0f;

			for (auto& pv : light.position_variations.value) {
				pv *= positional_vibration;
			}

			light.variation.value *= intensity_vibration;
			light.wall_variation.value *= intensity_vibration;
		}

	}
	else if constexpr(std::is_same_v<N, editor_wandering_pixels_node>) {
		auto& wandering_pixels = agg.template get<components::wandering_pixels>();
		wandering_pixels = static_cast<const components::wandering_pixels&>(editable);
		wandering_pixels.num_particles = std::min(MAX_WANDERING_PIXELS, wandering_pixels.num_particles);
	}
	else if constexpr(std::is_same_v<N, editor_particles_node>) {
		auto& cp = agg.template get<components::continuous_particles>();
		cp.modifier = static_cast<const particle_effect_modifier&>(editable);
		cp.modifier.sanitize();
	}
	else if constexpr(is_one_of_v<N, editor_point_marker_node, editor_area_marker_node>) {
		auto& marker = agg.template get<components::marker>();
		marker.faction = editable.faction;
		marker.letter = editable.letter;

		if constexpr(std::is_same_v<N, editor_area_marker_node>) {
			if (::is_portal_based(resource.editable.type)) {
				dependent_on_other_nodes = true;

				if (const auto portal = agg.template find<components::portal>()) {
					auto& to = *portal;
					auto from = editable.as_portal;

					if (from.color_preset != editor_color_preset::CUSTOM) {
						from.apply(from.color_preset);
					}

					to.exit_cooldown_ms = from.exit_cooldown_ms;

					to.ignore_airborne_characters = from.ignore_airborne_characters;
					to.ignore_walking_characters = from.ignore_walking_characters;

					to.enter_time_ms = from.enter_time_ms;
					to.travel_time_ms = from.travel_time_ms;

					to.force_field = from.force_field;
					to.hazard = from.hazard;

					if (from.disable_all_entry_effects) {
						to.light_size_mult = 0.0f;

						to.begin_entering_highlight_ms = 0;
						to.rings_effect.is_enabled = false;
						to.decrease_opacity_to = 255;
					}
					else {
						to.light_size_mult = from.light_size_mult;
						to.light_color = from.light_color;

						to.begin_entering_highlight_ms = from.begin_entering_highlight_ms;
						to.rings_effect = from.rings_effect;

						to.enter_shake = from.enter_shake;

						to.begin_entering_sound = to_game_effect(from.begin_entering_sound);
						to.enter_sound = to_game_effect(from.enter_sound);

						to.begin_entering_particles = to_game_effect(from.begin_entering_particles);
						to.enter_particles = to_game_effect(from.enter_particles);
						to.decrease_opacity_to = from.decrease_opacity_to;

						if (from.auto_scale_pitches) {
							const auto max_pitch = 2.5f;

							if (to.enter_time_ms != 0.0f) {
								to.begin_entering_sound.modifier.pitch *= std::min(max_pitch, 1.0f / (to.enter_time_ms / 500.0f));
							}

							if (to.travel_time_ms != 0.0f) {
								to.enter_sound.modifier.pitch *= std::min(max_pitch, 1.0f / (to.travel_time_ms / 500.0f));
							}
						}

						if (const auto particles = agg.template find<components::continuous_particles>()) {
							particles->modifier = static_cast<const particle_effect_modifier&>(from.ambience_particles);
							particles->modifier.sanitize();

							// for now always force rectangular particles for HAZARD because rectangular LAVA CIRCLE looks better (I know ugly)
							if (resource.editable.type == area_marker_type::HAZARD || editable.shape == marker_shape_type::BOX) {
								const auto basic_area = 128 * 128 * PI<float>;
								const auto this_area = editable.size.area() / 1.4f;

								particles->modifier.box = editable.size;
								particles->modifier.scale_amounts *= this_area / basic_area;
							}
							else {
								const auto radius = float(editable.size.smaller_side()) / 2;
								const auto basic_radius = 128.0f;

								particles->modifier.radius = radius;
								particles->modifier.scale_amounts *= radius / basic_radius;
							}
						}
					}

					if (from.disable_all_exit_effects) {
						to.exit_impulses.set_zero();
						to.exit_highlight_ms = 0.0f;
					}
					else {
						to.exit_impulses = from.exit_impulses;

						to.exit_sound = to_game_effect(from.exit_sound);
						to.exit_particles = to_game_effect(from.exit_particles);
						to.exit_shake = from.exit_shake;
						to.exit_highlight_ms = from.exit_highlight_ms;
					}

					to.exit_position = from.exit_position;
					to.exit_direction = from.exit_direction;

					if (from.trampoline_like) {
						to.travel_time_ms = 0.f;
						to.enter_time_ms = 0.f;

						to.enter_shake = sentience_shake::zero();
						to.enter_sound = {};
						to.enter_particles = {};
					}

					to.custom_filter = filters[predefined_filter_type::PORTAL];
					to.custom_filter.maskBits = from.reacts_to.get_mask_bits(); 
					to.reacts_to_factions = from.reacts_to_factions;
				}
			}

			if (const auto marker = agg.template find<components::marker>()) {
				marker->shape = editable.shape;
			}
		}
	}

	if (auto geo = agg.template find<components::overridden_geo>()) {
		if constexpr(has_size_v<Editable>) {
			geo->size = editable.size;
		}

		if constexpr(has_flip_v<Editable>) {
			geo->flip.horizontally = editable.flip_horizontally;
			geo->flip.vertically = editable.flip_vertically;
		}
	}

	handle.set_logic_transform(node.get_transform());
	(void)resource;

	return dependent_on_other_nodes;
}

template <class N, class R, class H>
void setup_entity_from_node_post_construct(
	const N& node, 
	const R& resource,
	H& handle
) {
	(void)resource;

	if constexpr(std::is_same_v<N, editor_sprite_node>) {
		if (auto anim = handle.template find<components::animation>()) {
			if (!node.editable.randomize_starting_animation_frame) {
				auto& override_start = node.editable.starting_animation_frame;

				if (override_start.is_enabled) {
					anim->state.frame_num = override_start.value;
				}
				else {
					anim->state.frame_num = 0;
				}
			}

			if (node.editable.randomize_color_wave_offset) {
				if (auto spr_inv = handle.template find<invariants::sprite>()) {
					if (auto spr = handle.template find<components::sprite>()) {
						if (spr_inv->has_color_wave()) {
							spr->effect_offset_secs = handle.get_cosmos().get_rng_for(handle.get_id()).randval(0.0f, 20.0f);
						}
					}
				}
			}	

			anim->speed_factor *= node.editable.animation_speed_factor;
		}
	}
}

inline real32 editor_light_falloff::calc_attenuation_mult_for_requested_radius() const {
	/*
		Taking component defaults is kinda stupid but we have to bear with it until we can remove legacy maps
		Ultimately vibration might scale with the values itself
		Or we'll just have hardcoded percentage like 10% and it will scale that

		Update:
		Let's actually not consider vibration in light range calculations
		vibration is meant to be minor
		also it should vibrate to the smaller side (so more attenuation)

		(void)vibration;
	*/

	const auto atten_at_edge = 
		constant +
		linear * radius +
		quadratic * radius * radius
	;

	if (atten_at_edge == 0.0f) {
		return 1.0f;
	}

	/*
		Strength is just another name for cutoff alpha.
	*/

	const auto cutoff_alpha = strength;
	return 255.f / (atten_at_edge * float(std::max(rgba_channel(1), cutoff_alpha)));
}

inline uint16_t editor_filter_flags::get_mask_bits() const {
	using C = filter_category;

	auto flags = augs::enum_bitset<C>();

	auto set = [&flags](const bool f, const C c) {
		if (f) {
			flags.set(c);
		}
	};

	set(characters, C::CHARACTER);
	set(character_weapons, C::CHARACTER_WEAPON);
	set(bullets, C::FLYING_BULLET);
	set(flying_explosives, C::FLYING_EXPLOSIVE);
	set(flying_melees, C::FLYING_MELEE);
	set(lying_items, C::LYING_ITEM);
	set(shells, C::SHELL);
	set(obstacles, C::WALL);

	flags.set(C::QUERY);

	return static_cast<uint16>(flags.to_ulong());
}
