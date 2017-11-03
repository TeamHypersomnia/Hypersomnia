#include "augs/templates/container_templates.h"
#include "augs/audio/sound_buffer.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/sound_system.h"

#include "view/viewables/loaded_sounds.h"

#include "augs/audio/audio_settings.h"

void sound_system::clear() {
	per_entity_cache.clear();
	fading_sources.clear();
}

void sound_system::clear_sources_playing(const assets::sound_buffer_id id) {
	erase_if(fading_sources, [id](fading_source& source) {
		return id == source.id;
	});
	
	erase_if(per_entity_cache, [id](auto& it) {
		return id == it.second.recorded_component.input.effect.id;
	});
}

void sound_system::clear_dead_entities(const cosmos& new_cosmos) {
	std::vector<entity_id> to_erase;

	for (const auto& it : per_entity_cache) {
		if (
			new_cosmos[it.first].dead() 
			|| !new_cosmos[it.first].has<components::processing>()
			|| !new_cosmos[it.first].get<components::processing>().is_in(processing_subjects::WITH_SOUND_EXISTENCE)
		) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		const auto& effect = per_entity_cache[it].recorded_component.input.effect;

		if (effect.modifier.fade_on_exit) {
			const auto buffer_id = effect.id;
			fading_sources.push_back({ buffer_id, std::move(per_entity_cache[it].source) });
		}

		per_entity_cache.erase(it);
	}
}

sound_system::cache& sound_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

const sound_system::cache& sound_system::get_cache(const const_entity_handle id) const {
	return per_entity_cache.at(id.get_id());
}

void sound_system::update_listener(
	const const_entity_handle listener,
	const interpolation_system& sys
) {
	const auto si = listener.get_cosmos().get_si();
	const auto listener_pos = listener.get_viewing_transform(sys).pos;

	augs::set_listener_position(si, listener_pos);
	augs::set_listener_velocity(si, listener.get_effective_velocity());
	augs::set_listener_orientation({ 0.f, -1.f, 0.f, 0.f, 0.f, -1.f });
}

void sound_system::track_new_sound_existences_near_camera(
	const augs::audio_volume_settings& settings,
	const loaded_sounds& manager,
	const camera_cone cone,
	const vec2 screen_size,
	const const_entity_handle listening_character,
	const interpolation_system& sys
) {
#if 0
	auto queried_size = cone.visible_world_area;
	queried_size.set(10000.f, 10000.f);
#endif

	update_listener(listening_character, sys);

	const auto& cosmos = listening_character.get_cosmos();
	const auto listener_pos = listening_character.get_viewing_transform(sys).pos;
	const auto si = cosmos.get_si();

	cosmos.for_each(
		processing_subjects::WITH_SOUND_EXISTENCE, 
		[&](const const_entity_handle it) {
			auto& cache = get_cache(it);
			const auto& existence = it.get<components::sound_existence>();
			auto& source = cache.source;

			const auto buffer_id = existence.input.effect.id;
			const auto& buffer = manager.at(buffer_id).variations[existence.input.variation_number];

			const auto& requested_buf = 
				existence.input.direct_listener == listening_character ? 
				buffer.stereo_or_mono() : 
				buffer.mono_or_stereo()
			;

			if (const bool refresh_cache =
				cache.recorded_component.time_of_birth != existence.time_of_birth
				|| cache.recorded_component.input.effect.id != existence.input.effect.id
				|| &requested_buf != source.get_bound_buffer()
			) {
				if (source.is_playing() && cache.recorded_component.input.effect.modifier.fade_on_exit) {
					fading_sources.push_back({ buffer_id, std::move(source) });

					source = augs::sound_source();
				}

				if (listening_character == existence.input.direct_listener) {
					source.bind_buffer(buffer.stereo_or_mono());
					source.set_direct_channels(true);
				}
				else {
					source.bind_buffer(buffer.mono_or_stereo());
					source.set_direct_channels(false);
				}

				source.play();
				source.set_max_distance(si, existence.input.effect.modifier.max_distance);
				source.set_reference_distance(si, existence.input.effect.modifier.reference_distance);
				source.set_looping(existence.input.effect.modifier.repetitions == -1);

				cache.recorded_component = existence;
			}

			const auto source_pos = it.get_viewing_transform(sys).pos;
			const auto dist_from_listener = (listener_pos - source_pos).length();
			const float absorption = std::min(10.f, static_cast<float>(pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2)));

			source.set_air_absorption_factor(absorption);
			source.set_pitch(existence.input.effect.modifier.pitch);
			source.set_gain(existence.input.effect.modifier.gain * settings.sound_effects);
			source.set_position(si, source_pos);
			source.set_velocity(si, it.get_effective_velocity());
		}
	);
}

void sound_system::fade_sources(const augs::delta dt) {
	erase_if(fading_sources, [dt](fading_source& f) {
		auto& source = f.source;

		const auto new_gain = source.get_gain() - dt.in_seconds()*3.f;
		const auto new_pitch = source.get_pitch() - dt.in_seconds()/3.f;

		if (new_pitch > 0.1f) {
			source.set_pitch(new_pitch);
		}

		if (new_gain > 0.f) {
			source.set_gain(new_gain);
			return false;
		}
		else {
			return true;
		}
	});
}
