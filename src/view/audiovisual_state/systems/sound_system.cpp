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
#include "game/detail/gun/firearm_engine.h"

#include "view/viewables/loaded_sounds_map.h"

#include "augs/audio/audio_settings.h"

std::optional<transformr> sound_system::update_properties_input::find_transform(const absolute_or_local& positioning) const {
	return ::find_transform(positioning, get_listener().get_cosmos(), interp);
}

void sound_system::clear() {
	short_sounds.clear();
	fading_sources.clear();
}

void sound_system::start_fading(generic_sound_cache& cache, const float fade_per_sec) {
	auto& source = cache.source;

	if (source.is_playing()) {
		fading_sources.push_back({ cache.original.input.id, std::move(source), fade_per_sec });
	}
}

void sound_system::clear_sources_playing(const assets::sound_id id) {
	erase_if(fading_sources, [id](const fading_source& source) {
		return id == source.id;
	});
	
	auto erase_pred = [id](const generic_sound_cache& it) {
		return id == it.original.input.id;
	};

	erase_if(short_sounds, erase_pred);
	erase_if(firearm_engine_caches, [erase_pred](const auto& it) {
		return erase_pred(it.second);
	});
}

void sound_system::clear_dead_entities(const cosmos& new_cosmos) {
	erase_if(short_sounds, [&](generic_sound_cache& it) {
		const auto target = it.positioning.target;

		if (target.is_set()) {
			if (new_cosmos[target].dead()) {
				if (!it.previous_transform) {
					return true;
				}

				it.positioning.offset = *it.previous_transform;
				it.positioning.target = {};
			}
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

void sound_system::generic_sound_cache::bind(const augs::sound_buffer& buf, const entity_id listener) {
	const bool is_direct_listener = listener == original.start.direct_listener;
	source.bind_buffer(buf, original.start.variation_number, is_direct_listener);
}

bool sound_system::generic_sound_cache::rebind_buffer(const update_properties_input in) {
	if (auto buf = mapped_or_nullptr(in.manager, original.input.id)) {
		bind(*buf, in.get_listener());
		return true;
	}

	return false;
}

bool sound_system::generic_sound_cache::update_properties(const update_properties_input in) {
	const auto listening_character = in.get_listener();
	const bool is_direct_listener = listening_character == original.start.direct_listener;

	const auto listener_pos = listening_character.get_viewing_transform(in.interp).pos;

	const auto& cosm = listening_character.get_cosmos();
	const auto si = cosm.get_si();
	const auto maybe_transform = in.find_transform(positioning);

	if (!maybe_transform) {
		source.set_gain(0.f);
	}

	const auto current_transform = *maybe_transform;

	if (is_direct_listener) {
		source.set_air_absorption_factor(0.f);
	}
	else {
		const auto dist_from_listener = (listener_pos - current_transform.pos).length();
		const auto absorption = std::min(10.f, static_cast<float>(pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2)));
		source.set_air_absorption_factor(absorption);
	}

	if (previous_transform && !is_direct_listener) {
		const auto displacement = current_transform - *previous_transform;
		previous_transform = current_transform;

		const auto effective_velocity = displacement.pos * in.dt.in_seconds();
		source.set_velocity(si, effective_velocity);
	}

	const auto& input = original.input;
	const auto& m = input.modifier;

	source.set_pitch(m.pitch);
	source.set_gain(m.gain * in.settings.sound_effects);
	source.set_max_distance(si, m.max_distance);
	source.set_reference_distance(si, m.reference_distance);
	source.set_looping(m.repetitions == -1);

	if (is_direct_listener) {
		source.set_relative_and_zero_vel_pos();
	}
	else {
		source.set_position(si, current_transform.pos);
	}

	return !source.is_playing();
}

void sound_system::update_effects_from_messages(const const_logic_step step, const update_properties_input in) {
	{
		const auto& events = step.get_queue<messages::stop_sound_effect>();

		for (auto& e : events) {
			erase_if(short_sounds, [&](generic_sound_cache& c){	
				if (const auto m = e.match_chased_subject) {
					if (*m != c.positioning.target) { 
						return false;
					}
				}

				if (const auto m = e.match_effect_id) {
					if (*m != c.original.input.id) {
						return false;
					}	
				}

				if (c.original.input.modifier.fade_on_exit) {
					start_fading(c);
				}

				return true;
			});
		}
	}

	const auto& events = step.get_queue<messages::start_sound_effect>();

	for (auto& e : events) {
		try {
			short_sounds.emplace_back(e.payload, in);
		}
		catch (const effect_not_found&) {

		}
		catch (const augs::too_many_sound_sources_error& err) {
			LOG("Warning: maxmimum number of sound sources reached at sound_system.cpp.");
		}
	}
}

void sound_system::update_sound_properties(const update_properties_input in) {
#if 0
	auto queried_size = cone.visible_world_area;
	queried_size.set(10000.f, 10000.f);
#endif
	const auto listening_character = in.get_listener();

	update_listener(listening_character, in.interp);

	const auto& cosm = listening_character.get_cosmos();

	cosm.for_each_having<components::gun>(
		[&](const auto gun_entity) {
			const auto id = gun_entity.get_id().to_unversioned();

			if (const auto sound = ::calc_firearm_engine_sound(gun_entity)) {
				if (auto* const existing = mapped_or_nullptr(firearm_engine_caches, id)) {
					existing->original = *sound;

					if (!existing->rebind_buffer(in)) {
						fade_and_erase(firearm_engine_caches, id);
					}
				}
				else {
					try {
						firearm_engine_caches.try_emplace(id, *sound, in);
					}
					catch (...) {

					}
				}
			}
			else {
				fade_and_erase(firearm_engine_caches, id);
			}
		}
	);

	erase_if(firearm_engine_caches, [&](auto& it) {
		auto& cache = it.second;

		if (!can_have_firearm_engine_effect(cosm[it.first])) {
			start_fading(cache);
			return true;
		}

		return cache.update_properties(in);
	});

	erase_if(short_sounds, [&](generic_sound_cache& cache) {
		return cache.update_properties(in);
	});
}

void sound_system::fade_sources(const augs::delta dt) {
	erase_if(fading_sources, [dt](fading_source& f) {
		auto& source = f.source;

		const auto new_gain = source.get_gain() - dt.in_seconds() * f.fade_per_sec;
		const auto new_pitch = source.get_pitch() - dt.in_seconds() * f.fade_per_sec;

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
