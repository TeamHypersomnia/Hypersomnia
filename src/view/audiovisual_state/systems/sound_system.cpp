#include "augs/templates/container_templates.h"
#include "augs/audio/sound_buffer.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/messages/start_sound_effect.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/sound_system.h"

#include "view/viewables/loaded_sounds.h"

#include "augs/audio/audio_settings.h"

entity_id get_target_if_any(const absolute_or_local& l) {
	if (const auto chasing = std::get_if<orbital_chasing>(&l)) {
		return chasing->target;
	}

	return {};
}

void sound_system::clear() {
	short_sounds.clear();
	fading_sources.clear();
}

void sound_system::clear_sources_playing(const assets::sound_buffer_id id) {
	erase_if(fading_sources, [id](fading_source& source) {
		return id == source.id;
	});
	
	erase_if(short_sounds, [id](short_sound_cache& it) {
		return id == it.original_input.id;
	});
}

void sound_system::clear_dead_entities(const cosmos& new_cosmos) {
	erase_if(short_sounds, [&](short_sound_cache& it) {
		if (new_cosmos[get_target_if_any(it.positioning)].dead()) {
			if (!it.previous_transform) {
				return true;
			}

			it.positioning = *it.previous_transform;
		}

		return false;
	});
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

void sound_system::update_effects_from_messages(
	const_logic_step step,
	const loaded_sounds& manager,
	const interpolation_system& interp,
	const viewer_eye ear
) {
	const auto listening_character = ear.viewed_character;
	const auto& cosmos = listening_character.get_cosmos();
	const auto si = cosmos.get_si();

	{
		const auto& events = step.get_queue<messages::stop_sound_effect>();

		for (auto& e : events) {
			erase_if(short_sounds, [&](short_sound_cache& c){	
				if (const auto m = e.match_chased_subject) {
					if (*m != get_target_if_any(c.positioning)) { 
						return false;
					}
				}

				if (const auto m = e.match_effect_id) {
					if (*m != c.original_input.id) {
						return false;
					}	
				}

				if (const auto& effect = c.original_input;
					effect.modifier.fade_on_exit
				) {
					auto& source = c.source;

					if (source.is_playing()) {
						fading_sources.push_back({ effect.id, std::move(source) });
					}
				}

				return true;
			});
		}
	}

	const auto& events = step.get_queue<messages::start_sound_effect>();

	for (auto& e : events) {
		short_sounds.emplace_back();
		auto& cache = short_sounds.back();

		cache.original_input = e.effect;
		cache.original_start = e.start;

		cache.positioning = e.start.positioning;
		cache.previous_transform = find_transform(e.start.positioning, cosmos, interp);

		auto& source = cache.source;

		{
			const auto effect_id = e.effect.id;

			const auto& variations = manager.at(effect_id).variations;
			const auto chosen_variation = e.start.variation_number % variations.size();
			const auto& buffer = variations[chosen_variation];

			const bool is_direct_listener = listening_character == e.start.direct_listener;

			const auto& requested_buf = 
				is_direct_listener ? buffer.stereo_or_mono() : buffer.mono_or_stereo()
			;

			source.bind_buffer(requested_buf);
			source.set_direct_channels(is_direct_listener);
		}

		source.play();

		const auto& modifier = e.effect.modifier;

		source.set_max_distance(si, modifier.max_distance);
		source.set_reference_distance(si, modifier.reference_distance);
		source.set_looping(modifier.repetitions == -1);
	}
}

void sound_system::update_sound_properties(
	const augs::audio_volume_settings& settings,
	const loaded_sounds& manager,
	const interpolation_system& interp,
	const viewer_eye ear,
	const augs::delta dt
) {
#if 0
	auto queried_size = cone.visible_world_area;
	queried_size.set(10000.f, 10000.f);
#endif

	const auto listening_character = ear.viewed_character;

	update_listener(listening_character, interp);

	const auto& cosmos = listening_character.get_cosmos();
	const auto si = cosmos.get_si();

	const auto listener_pos = listening_character.get_viewing_transform(interp).pos;

	erase_if(short_sounds, [&](short_sound_cache& cache) {
		const auto& positioning = cache.positioning;
		const auto maybe_transform = find_transform(positioning, cosmos, interp);

		auto& source = cache.source;

		if (!maybe_transform) {
			source.set_gain(0.f);
		}

		const auto current_transform = *maybe_transform;

		{
			const auto dist_from_listener = (listener_pos - current_transform.pos).length();
			const float absorption = std::min(10.f, static_cast<float>(pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2)));

			source.set_air_absorption_factor(absorption);
		}

		{
			if (cache.previous_transform) {
				const auto displacement = current_transform - *cache.previous_transform;
				cache.previous_transform = current_transform;

				const auto effective_velocity = displacement.pos * dt.in_seconds();
				source.set_velocity(si, effective_velocity);
			}
		}

		const auto& input = cache.original_input;

		source.set_pitch(input.modifier.pitch);
		source.set_gain(input.modifier.gain * settings.sound_effects);
		source.set_position(si, current_transform.pos);

		return !source.is_playing();
	});
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

		return true;
	});
}
