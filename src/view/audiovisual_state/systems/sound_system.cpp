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
#include "game/detail/sentience/sentience_getters.h"

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
	if (container_full(fading_sources)) {
		return;
	}

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
		return linear_erase(it.second.cache);
	};

	erase_if(short_sounds, linear_erase);
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
		original.input.modifier.always_direct_listener 
		|| listening_character == original.start.direct_listener
		|| (target_faction != faction_type::SPECTATOR && faction == target_faction)
	;

	const auto& cosm = listening_character.get_cosmos();
	const auto si = cosm.get_si();
	const auto maybe_transform = in.find_transform(positioning);

	if (!maybe_transform) {
		source.set_gain(0.f);
	}

	const auto current_transform = *maybe_transform;

	const auto& common = cosm.get_common_significant();
	const auto& defaults = common.default_sound_properties;

	if (is_direct_listener) {
		source.set_air_absorption_factor(0.f);
	}
	else {
#if 0
		const auto listener_pos = listening_character.get_viewing_transform(in.interp).pos;

		const auto dist_from_listener = (listener_pos - current_transform.pos).length();
		const auto absorption = 1.f + 0 * std::min(10.f, 3 * static_cast<float>(pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2)));
#endif
		source.set_air_absorption_factor(std::clamp(defaults.air_absorption, 0.f, 10.f));
	}

	if (previous_transform && !is_direct_listener && !(in.dt == augs::delta::zero)) {
		const bool interp_enabled = in.interp.is_enabled();
		const auto frame_dt_in_steps = in.dt.in_steps_per_second();

		const auto effective_velocity = [&]() {
			const auto displacement = current_transform.pos - previous_transform->pos;

			if (interp_enabled) {
				return displacement * frame_dt_in_steps;
			}

			return displacement * cosm.get_fixed_delta().in_steps_per_second();
		}();

		previous_transform = current_transform;

		if (interp_enabled || when_set_velocity != cosm.get_timestamp()) {
			source.set_velocity(si, effective_velocity);
		}

		when_set_velocity = cosm.get_timestamp();
	}

	const auto& input = original.input;
	const auto& m = input.modifier;

	auto max_dist = m.max_distance;
	auto ref_dist = m.reference_distance;
	auto dist_model = m.distance_model;

	if (dist_model == augs::distance_model::NONE) {
		dist_model = defaults.distance_model;
	}

	if (max_dist < 0.f) {
		max_dist = defaults.max_distance;
	}

	if (ref_dist < 0.f) {
		ref_dist = defaults.reference_distance;
	}

	const bool is_linear = 
		dist_model == augs::distance_model::LINEAR_DISTANCE
		|| dist_model == augs::distance_model::LINEAR_DISTANCE_CLAMPED
	;

	const auto mult_via_settings = [&]() {
		if (original.input.modifier.always_direct_listener) {
			if (const auto buf = source.get_bound_buffer()) {
				if (buf->get_length_in_seconds() > in.settings.treat_as_music_sounds_longer_than_secs) {
					return in.volume.music;
				}
			}
		}

		return in.volume.sound_effects;
	}();

	source.set_pitch(m.pitch * in.speed_multiplier);
	source.set_gain(std::clamp(m.gain, 0.f, 1.f) * mult_via_settings);
	source.set_reference_distance(si, ref_dist);
	source.set_looping(m.repetitions == -1);
	source.set_distance_model(dist_model);
	source.set_doppler_factor(std::max(0.f, m.doppler_factor));

	if (is_linear) {
		source.set_max_distance(si, max_dist);
	}
	else {
		/* 
			rolloff does not make sense for linear models. 
			Simply decrease the max_distance parameter instead.

			Here we estimate the rolloff factor based on what we want as the max_distance.
		*/

		source.set_rolloff_factor(std::max(0.f, defaults.basic_nonlinear_rolloff * (ref_dist / max_dist)));
	}

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

	auto do_events = [&](auto* dummy) {
		using M = remove_cptr<decltype(dummy)>;

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
				if (!container_full(short_sounds)) {
					short_sounds.emplace_back(e.payload, in);
				}
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

	do_events((messages::start_sound_effect*)(nullptr));
	do_events((messages::start_multi_sound_effect*)(nullptr));
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
					existing->cache.original = *sound;

					if (!existing->cache.rebind_buffer(in)) {
						fade_and_erase(firearm_engine_caches, id);
					}
				}
				else {
					try {
						firearm_engine_caches.try_emplace(id, continuous_sound_cache { { *sound, in }, { gun_entity.get_name() } });
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
				existing->cache.original = sound;

				if (!existing->cache.rebind_buffer(in)) {
					fade_and_erase(continuous_sound_caches, id);
				}
			}
			else {
				try {
					continuous_sound_caches.try_emplace(id, continuous_sound_cache { { sound, in }, { sound_entity.get_name() } } );
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
		const auto& recorded = it.second.recorded;
		const auto handle = cosm[it.first];

		auto& cache = it.second.cache;

		if (handle.dead()) {
			start_fading(cache);
			return true;
		}

		if (recorded.name != handle.get_name()) {
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

	auto reseek = [&](const auto& subject, const auto& cache) {
		const auto& source = cache.source;
		const auto& m = cache.original.input.modifier;

		if (const auto buf = source.get_bound_buffer()) {
			const auto secs = buf->get_length_in_seconds();

			if (secs > in.settings.sync_sounds_longer_than_secs) {
				const auto when_born = subject.when_born().step;
				const auto now_step = subject.get_cosmos().get_timestamp().step;

				if (now_step < when_born) {
					return;
				}

				const auto total_lived_secs = (now_step - when_born) * in.inv_tickrate * in.speed_multiplier;
				const auto total_lived_cycles = total_lived_secs / secs;

				const auto expected_secs = [&]() {
					if (m.repetitions == -1) {
						return std::fmod(total_lived_secs, secs);
					}

					if (total_lived_cycles + 1 >= m.repetitions) {
						return std::min(total_lived_secs, secs);
					}

					return std::fmod(total_lived_secs, secs);
				}();

				const auto max_divergence = in.settings.max_divergence_before_sync_secs;
				const auto actual_secs = source.get_time_in_seconds();

				//LOG_NVPS(subject, expected_secs, actual_secs);

				if (std::abs(expected_secs - actual_secs) > max_divergence) {
					source.seek_to(expected_secs);
				}
			}
		}
	};

	erase_if(continuous_sound_caches, [&](auto& it) {
		const auto& recorded = it.second.recorded;
		const auto handle = cosm[it.first];

		auto& cache = it.second.cache;

		if (handle.dead()) {
			start_fading(cache);
			return true;
		}

		if (!can_have_continuous_sound(handle)) {
			start_fading(cache);
			return true;
		}

		if (recorded.name != handle.get_name()) {
			start_fading(cache);
			return true;
		}

		const auto result = update_facade(cache);

		if (!result) {
			reseek(cosm[it.first], cache);
		}

		return result;
	});

	erase_if(short_sounds, [&](generic_sound_cache& cache) {
		const auto& in = cache.original.start;

		const auto logical_subject = cosm[in.positioning.target];

		auto request_erase = [&]() {
			start_fading(cache);
			return true;
		};

		if (!logical_subject) {
			if (in.clear_when_target_entity_deleted) {
				return request_erase();
			}
		}
		else {
			if (in.clear_when_target_alive) {
				if (::sentient_and_alive(logical_subject)) {
					return request_erase();
				}
			}

			if (in.clear_when_target_conscious) {
				if (::sentient_and_conscious(logical_subject)) {
					return request_erase();
				}
			}
		}

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
