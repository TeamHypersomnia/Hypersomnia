#pragma once
#include "game/detail/gun/gun_math.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

template <class T>
auto calc_firearm_engine_particles(const T& gun_handle)
	-> std::optional<packaged_particle_effect>
{
	static_assert(T::template has<components::gun>());
	static_assert(T::template has<invariants::gun>());

	auto& gun = gun_handle.template get<components::gun>();
	auto& gun_def = gun_handle.template get<invariants::gun>();

	packaged_particle_effect particles;

	if (gun.current_heat <= 0.05f) {
		return std::nullopt;
	}

	{
		const auto mult = gun.current_heat / gun_def.maximum_heat;

		auto in = gun_def.firing_engine_particles;

		in.modifier.scale_amounts = std::min(1.f, mult + in.modifier.scale_amounts);
		//in.modifier.scale_lifetimes += mult;

		particles.input = in;
	}

	particles.start = particle_effect_start_input::orbit_local(
		gun_handle, calc_muzzle_transform(gun_handle, {})
	);

	particles.start.stream_infinitely = true;

	return particles;
}

template <class T>
auto calc_firearm_engine_sound(const T& gun_handle)
	-> std::optional<packaged_sound_effect>
{
	static_assert(T::template has<components::gun>());
	static_assert(T::template has<invariants::gun>());

	auto& gun = gun_handle.template get<components::gun>();
	auto& gun_def = gun_handle.template get<invariants::gun>();

	const bool effect_enabled = gun.current_heat > 0.20f;

	if (effect_enabled) {
		packaged_sound_effect sound;

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

		return sound;
	}

	return std::nullopt;
}
