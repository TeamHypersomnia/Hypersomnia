#pragma once
#include "game/components/gun_component.h"
#include "game/components/sprite_component.h"

template <class T>
vec2i get_bullet_spawn_offset(const T& gun_handle) {
	const auto& cosmos = gun_handle.get_cosmos();

	if (const auto* const sprite = gun_handle.template find<invariants::sprite>()) {
		const auto reference_id = sprite->image_id;
		const auto& offsets = cosmos.get_logical_assets().get_offsets(reference_id);

		return offsets.gun.bullet_spawn;
	}

	return {};
}

template <class T>
vec2 calc_muzzle_position(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	if (const auto logical_width = gun_handle.find_logical_width()) {
		const auto muzzle_transform = gun_transform * transformr(bullet_spawn_offset + vec2(*logical_width / 2, 0));
		return muzzle_transform.pos;
	}

	return {};
}

template <class T>
vec2 calc_barrel_center(
	const T& gun_handle,
	const transformr& gun_transform
) {
	const auto bullet_spawn_offset = get_bullet_spawn_offset(gun_handle);

	return (gun_transform * transformr(vec2(0, bullet_spawn_offset.y))).pos;
}

template <class T>
auto can_have_firearm_engine_effect(const T& gun_handle) {
	return gun_handle.template has<components::gun>();
}

template <class T>
auto calc_firearm_engine_effects(const T& gun_handle)
	-> std::optional<packaged_particles_and_sound>
{
	static_assert(T::template has<components::gun>());
	static_assert(T::template has<invariants::gun>());

	auto& gun = gun_handle.template get<components::gun>();
	auto& gun_def = gun_handle.template get<invariants::gun>();

	const bool effect_enabled = gun.current_heat > 0.20f;

	if (effect_enabled) {
		packaged_particles_and_sound result;

		{
			auto& sound = result.sound;

			auto total_pitch = static_cast<float>(gun.current_heat / gun_def.maximum_heat);
			auto total_gain = (gun.current_heat - 0.20f) / gun_def.maximum_heat;

			total_pitch *= total_pitch;
			total_gain *= total_gain;

			{
				auto in = gun_def.firing_engine_sound;

				in.modifier.pitch *= total_pitch;
				in.modifier.gain *= total_gain;
				in.modifier.repetitions = -1;

				sound.input = in;
			}

			const auto owning_capability = gun_handle.get_owning_transfer_capability();

			sound.start = sound_effect_start_input::at_entity(gun_handle).set_listener(owning_capability);
		}

		return result;
	}

	return std::nullopt;
}
