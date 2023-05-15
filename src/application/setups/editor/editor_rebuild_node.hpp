#pragma once

template <class T>
void make_unselectable(T& agg) {
	agg.when_born.step = 2;
}

template <class T>
void make_unselectable_handle(T handle) {
	::make_unselectable(handle.get({}));
}

template <class N, class R, class H, class A>
void setup_entity_from_node(
	const sorting_order_type total_order,
	const editor_layer& layer,
	const N& node, 
	const R& resource,
	H& handle, 
	A& agg
) {
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
