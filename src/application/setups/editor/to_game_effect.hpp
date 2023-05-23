#pragma once

template <class A, class R, class T>
auto convert_to_game_effect(A get_asset_id_of, R find_resource, const T& in) {
	auto do_full_modifier = [&](auto& out, auto& from) {
		out.gain = from.gain;
		out.pitch = from.pitch;
		out.max_distance = from.max_distance.is_enabled ? from.max_distance.value : -1;
		out.reference_distance = from.reference_distance.is_enabled ? from.reference_distance.value : -1;
		out.doppler_factor = from.doppler_intensity;
		out.distance_model = from.distance_model.is_enabled ? from.distance_model.value : augs::distance_model::NONE;
		out.always_direct_listener = !from.spatialize;

		if (from.loop) {
			out.repetitions = -1;
		}
		else {
			out.repetitions = from.play_times;
		}
	};

	if constexpr(std::is_same_v<T, editor_typed_resource_id<editor_sound_resource>>) {
		sound_effect_input out;
		out.id = get_asset_id_of(in);

		if (const auto parent_res = find_resource(in)) {
			const auto& resource_modifier = parent_res->editable;
			do_full_modifier(out.modifier, resource_modifier);
		}

		return out;
	}
	else if constexpr(std::is_same_v<T, editor_sound_effect>) {
		sound_effect_input out;
		out.id = get_asset_id_of(in.id);

		if (const auto parent_res = find_resource(in.id)) {
			const auto& resource_modifier = parent_res->editable;
			do_full_modifier(out.modifier, resource_modifier);
		}

		const auto& property_modifier = static_cast<const editor_sound_property_effect_modifier&>(in);

		out.modifier.gain *= property_modifier.gain;
		out.modifier.pitch *= property_modifier.pitch;

		return out;
	}
	else if constexpr(std::is_same_v<T, editor_sound_effect_modifier>) {
		sound_effect_modifier out;
		do_full_modifier(out, in);
		return out;
	}
	else if constexpr(std::is_same_v<T, editor_theme>) {
		sound_effect_input out;
		out.id = get_asset_id_of(in.id);

		out.modifier.gain = in.gain;
		out.modifier.pitch = in.pitch;
		out.modifier.always_direct_listener = true;
		out.modifier.repetitions = -1;

		return out;
	}
	else if constexpr(std::is_same_v<T, editor_particle_effect>) {
		particle_effect_input out;
		out.modifier = static_cast<particle_effect_modifier>(in);
		out.modifier.sanitize();
		out.id = get_asset_id_of(in.id);

		return out;
	}
}
