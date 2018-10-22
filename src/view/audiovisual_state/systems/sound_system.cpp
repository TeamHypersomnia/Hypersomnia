#include "augs/templates/container_templates.h"
#include "augs/audio/sound_buffer.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/messages/start_sound_effect.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/sound_system.h"
#include "game/detail/gun/firearm_engine.h"

#include "view/viewables/loaded_sounds_map.h"

#include "augs/audio/audio_settings.h"
#include "view/character_camera.h"

struct shouldnt_play {};

std::optional<transformr> sound_system::update_properties_input::find_transform(const absolute_or_local& positioning) const {
	return ::find_transform(positioning, get_listener().get_cosmos(), interp);
}

const_entity_handle sound_system::update_properties_input::get_listener() const {
	return ear.viewed_character;
}

void sound_system::clear() {
	short_sounds.clear();
	fading_sources.clear();
	firearm_engine_caches.clear();
	continuous_sound_caches.clear();
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
	
	auto linear_erase = [id](const generic_sound_cache& it) {
		return id == it.original.input.id;
	};

	auto map_erase = [linear_erase](const auto& it) {
		return linear_erase(it.second);
	};

	erase_if(short_sounds, linear_erase);
	erase_if(firearm_engine_caches, map_erase);
	erase_if(continuous_sound_caches, map_erase);
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

	auto map_erase = [&](const auto& it) {
		return new_cosmos[it.first].dead();
	};

	erase_if(firearm_engine_caches, map_erase);
	erase_if(continuous_sound_caches, map_erase);
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

void sound_system::generic_sound_cache::init(update_properties_input in) {
	if (!should_play(in)) {
		throw shouldnt_play {};
	}

	if (!rebind_buffer(in)) {
		throw effect_not_found {}; 
	}

	update_properties(in);
	previous_transform = in.find_transform(positioning);

	source.play();
}

sound_system::generic_sound_cache::generic_sound_cache(
	const packaged_sound_effect& original,
	const update_properties_input in
) : 
	original(original),
	positioning(original.start.positioning)
{
	init(in);
}

sound_system::generic_sound_cache::generic_sound_cache(
	const packaged_multi_sound_effect& multi,
	const update_properties_input in
) :
	positioning(multi.start.positioning),
	followup_inputs(multi.inputs)
{
	original.start = multi.start;

	if (multi.inputs.empty()) {
		throw effect_not_found {}; 
	}

	eat_followup();
	init(in);
}

void sound_system::generic_sound_cache::bind(const augs::sound_buffer& buf) {
	source.bind_buffer(buf, original.start.variation_number);
}

bool sound_system::generic_sound_cache::rebind_buffer(const update_properties_input in) {
	if (auto buf = mapped_or_nullptr(in.manager, original.input.id)) {
		bind(*buf);
		return true;
	}

	return false;
}

bool sound_system::generic_sound_cache::should_play(const update_properties_input in) const {
	const auto listening_character = in.get_listener();

	if (listening_character.dead()) {
		return false;
	}

	const auto faction = listening_character.get_official_faction();
	const auto target_faction = original.start.listener_faction;

	return target_faction == faction_type::SPECTATOR || faction == target_faction;
}

void sound_system::generic_sound_cache::eat_followup() {
	original.input = followup_inputs[0];
	followup_inputs.erase(followup_inputs.begin());
}

void sound_system::generic_sound_cache::update_properties(const update_properties_input in) {
	const auto listening_character = in.get_listener();

	if (listening_character.dead()) {
		return;
	}

	const auto faction = listening_character.get_official_faction();
	const auto target_faction = original.start.listener_faction;

	const bool is_direct_listener = 
		original.start.always_direct_listener 
		|| listening_character == original.start.direct_listener
		|| (target_faction != faction_type::SPECTATOR && faction == target_faction)
	;

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

	auto g_mult = 1.f;
	
	if (m.distance_model != augs::distance_model::LINEAR_DISTANCE
		&& m.distance_model != augs::distance_model::LINEAR_DISTANCE_CLAMPED
	) {
		/* Correct the gain of non-linear sources so that they eventually become silent. */
		const auto d = (listener_pos - current_transform.pos).length();

		const auto md = m.max_distance;
		const auto rd = m.reference_distance ;

		const auto thres = md - rd;

		if (rd > 0.f && d > thres) {
			g_mult = std::max(0.f, 1.f - ((d - thres) / rd));
		}
	}

	source.set_pitch(m.pitch * in.speed_multiplier);
	source.set_gain(g_mult * m.gain * in.settings.sound_effects);
	source.set_max_distance(si, std::max(0.f, m.max_distance));
	source.set_reference_distance(si, std::max(0.f, m.reference_distance));
	source.set_looping(m.repetitions == -1);
	source.set_distance_model(m.distance_model);
	source.set_doppler_factor(std::max(0.f, m.doppler_factor));
	source.set_rolloff_factor(std::max(0.f, m.rolloff_factor));

	source.set_spatialize(!is_direct_listener);
	source.set_direct_channels(is_direct_listener);

	if (is_direct_listener) {
		source.set_relative_and_zero_vel_pos();
	}
	else {
		source.set_position(si, current_transform.pos);
	}
}

void sound_system::generic_sound_cache::maybe_play_next(update_properties_input in) {
	if (source.is_playing()) {
		return;
	}

	if (followup_inputs.empty()) {
		return;
	}

	eat_followup();
	source.stop();

	if (rebind_buffer(in)) {
		update_properties(in);
		source.play();
	}
}

bool sound_system::generic_sound_cache::still_playing() const {
	return source.is_playing() || followup_inputs.size() > 0;
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

	auto do_events = [&](auto dummy) {
		using M = decltype(dummy);

		const auto& events = step.get_queue<M>();

		for (auto& e : events) {
			{
				const auto& start = e.payload.start;
				const auto& s = start.source_collision;

				if (!(s == collision_sound_source())) {
					const auto it = collision_sound_cooldowns.try_emplace(s);

					auto& cooldown = (*it.first).second;

					++cooldown.consecutive_occurences;
					++cooldown.remaining_ms = start.collision_sound_cooldown_duration;

					// LOG_NVPS(cooldown.consecutive_occurences);

					if (cooldown.consecutive_occurences > start.collision_sound_occurences_before_cooldown) {
						// LOG("Skipping");
						continue;
					}
				}
			}

			try {
				short_sounds.emplace_back(e.payload, in);
			}
			catch (const effect_not_found&) {

			}
			catch (const shouldnt_play&) {

			}
			catch (const augs::too_many_sound_sources_error& err) {
				LOG("Warning: maxmimum number of sound sources reached at sound_system.cpp.");
			}
		}
	};

	do_events(messages::start_sound_effect());
	do_events(messages::start_multi_sound_effect());
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

	cosm.for_each_having<invariants::continuous_sound>(
		[&](const auto sound_entity) {
			const auto id = sound_entity.get_id().to_unversioned();
			const auto& continuous_sound = sound_entity.template get<invariants::continuous_sound>();

			packaged_sound_effect sound;

			sound.start = sound_effect_start_input::at_entity(sound_entity);
			sound.input = continuous_sound.effect;

			if (auto* const existing = mapped_or_nullptr(continuous_sound_caches, id)) {
				existing->original = sound;

				if (!existing->rebind_buffer(in)) {
					fade_and_erase(continuous_sound_caches, id);
				}
			}
			else {
				try {
					continuous_sound_caches.try_emplace(id, sound, in);
				}
				catch (...) {

				}
			}
		}
	);

	auto update_facade = [&](auto& cache) {
		cache.update_properties(in);
		cache.maybe_play_next(in);

		return !cache.still_playing();
	};

	erase_if(firearm_engine_caches, [&](auto& it) {
		auto& cache = it.second;

		if (cosm[it.first].dead()) {
			start_fading(cache);
			return true;
		}

		return update_facade(cache);
	});

	auto can_have_continuous_sound = [&](const auto handle) {
		if (handle.dead()) {
			return false;
		}

		if (const auto c = handle.template find<invariants::continuous_sound>()) {
			return c->effect.id.is_set();
		}

		return false;
	};

	erase_if(continuous_sound_caches, [&](auto& it) {
		auto& cache = it.second;

		const auto handle = cosm[it.first];

		if (!can_have_continuous_sound(handle)) {
			start_fading(cache);
			return true;
		}

		return update_facade(cache);
	});

	erase_if(short_sounds, [&](generic_sound_cache& cache) {
		return update_facade(cache);
	});
}

void sound_system::fade_sources(const augs::delta dt) {
	erase_if (collision_sound_cooldowns, [&](auto& it) {
		auto& c = it.second;

		c.remaining_ms -= dt.in_milliseconds();

		if (c.remaining_ms <= 0.f) {
			return true;
		}

		return false;
	});

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
