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
#include "view/audiovisual_state/flashbang_math.h"
#include "game/detail/find_absolute_or_local_transform.h"
#include "augs/log.h"

struct shouldnt_play {};

void augs::update_multiple_properties::update(augs::sound_source_proxy_data& data) {
	data.last_pitch = pitch;
	data.last_gain = gain;
}

std::optional<transformr> sound_system::update_properties_input::find_transform(const absolute_or_local& positioning) const {
	return ::find_transform(positioning, get_listener().get_cosmos(), interp);
}

const_entity_handle sound_system::update_properties_input::get_listener() const {
	return ear.viewed_character;
}

bool sound_system::update_properties_input::under_short_sound_limit() const {
	return static_cast<int>(owner.short_sounds.size()) < settings.max_short_sounds;
}

void sound_system::clear() {
	short_sounds.clear();
	fading_sources.clear();
	firearm_engine_caches.clear();
	continuous_sound_caches.clear();
	collision_sound_cooldowns.clear();
	damage_sound_cooldowns.clear();
	id_pool.reset(SOUNDS_SOURCES_IN_POOL);
}

void sound_system::generic_sound_cache::stop_and_free(const update_properties_input& in) {
	{
		const auto proxy = get_proxy(in);
		proxy.stop();
	}

	in.owner.id_pool.free(source.id);
}

bool sound_system::start_fading(generic_sound_cache& cache, const float fade_per_sec) {
	if (!container_full(fading_sources)) {
		if (cache.probably_still_playing()) {
			fading_sources.push_back({ cache.original.input.id, cache.source, fade_per_sec });
			return true;
		}
	}

	return false;
}

void sound_system::clear_sources_playing(const assets::sound_id id) {
	auto and_free_proxy_id = [&](const augs::sound_source_proxy_id& id) {
		id_pool.free(id);
		return true;
	};

	erase_if(fading_sources, [&](const fading_source& f) {
		return id == f.id && and_free_proxy_id(f.source.id);
	});
	
	auto linear_erase = [id, and_free_proxy_id](const generic_sound_cache& it) {
		return id == it.original.input.id && and_free_proxy_id(it.source.id);
	};

	auto map_erase = [linear_erase](const auto& it) {
		return linear_erase(it.second.cache);
	};

	erase_if(short_sounds, linear_erase);
	erase_if(firearm_engine_caches, map_erase);
	erase_if(continuous_sound_caches, map_erase);
}

void sound_system::update_listener(
	const augs::audio_renderer& renderer,
	const const_entity_handle listener,
	const interpolation_system& sys,
	const sound_system_settings& settings,
	const vec2 world_screen_center
) {
	augs::update_listener_properties cmd;

	const auto character_transform = listener.get_viewing_transform(sys);

	{
		const auto character_pos = character_transform.pos;

		const auto listener_pos = 
			settings.listener_reference == listener_position_reference::SCREEN_CENTER ?
			world_screen_center :
			character_pos
		;

		cmd.position = listener_pos;
	}

	cmd.velocity = listener.get_effective_velocity();

	vec2 orientation = vec2(0, -1.f);

	if (settings.set_listener_orientation_to_character_orientation) {
		orientation = vec2(1, 0);
		orientation.rotate(character_transform.rotation);
	}

	cmd.si = listener.get_cosmos().get_si();
	cmd.orientation = orientation;
	
	renderer.push_command(cmd);
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

	const bool gain_dependent_lifetime = original.start.silent_trace_like;

	if (gain_dependent_lifetime) {
		return;
	}

	const auto proxy = get_proxy(in);
	proxy.play();
}

sound_system::generic_sound_cache::generic_sound_cache(
	const augs::sound_source_proxy_id proxy_id,
	const packaged_sound_effect& original,
	const update_properties_input in
) : 
	source(proxy_id),
	original(original),
	positioning(original.start.positioning)
{
	init(in);
}

sound_system::generic_sound_cache::generic_sound_cache(
	const augs::sound_source_proxy_id proxy_id,
	const packaged_multi_sound_effect& multi,
	const update_properties_input in
) :
	source(proxy_id),
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

augs::sound_source_proxy sound_system::generic_sound_cache::get_proxy(const update_properties_input& in) {
	return { in.renderer, source };
}

void sound_system::generic_sound_cache::bind(const update_properties_input& in, const augs::sound_buffer& buf) {
	const auto proxy = get_proxy(in);
	proxy.bind_buffer(buf, original.start.variation_number);
}

bool sound_system::generic_sound_cache::rebind_buffer(const update_properties_input in) {
	if (auto buf = mapped_or_nullptr(in.manager, original.input.id)) {
		bind(in, *buf);
		return true;
	}

	return false;
}

bool sound_system::generic_sound_cache::should_play(const update_properties_input in) const {
	const auto listening_character = in.get_listener();

	if (listening_character.dead()) {
		return false;
	}

	if (original.start.silent_trace_like) {
		if (original.start.direct_listener == listening_character) {
			return false;
		}
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

	const auto& input = original.input;
	const auto& m = input.modifier;

	const auto dt_this_frame = in.dt.in_seconds() * m.pitch;
	const bool gain_dependent_lifetime = original.start.silent_trace_like;

	if (!gain_dependent_lifetime) {
		elapsed_secs += dt_this_frame;
	}

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
	const auto maybe_transform = in.find_transform(positioning);

	augs::update_multiple_properties cmd;

	const auto current_transform = *maybe_transform;

	const auto& common = cosm.get_common_significant();
	const auto& defaults = common.default_sound_properties;

	if (is_direct_listener) {
		cmd.air_absorption_factor = 0.f;
	}
	else {
#if 0
		const auto listener_pos = listening_character.get_viewing_transform(in.interp).pos;

		const auto dist_from_listener = (listener_pos - current_transform.pos).length();
		const auto absorption = 1.f + 0 * std::min(10.f, 3 * static_cast<float>(pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2)));
#endif
		cmd.air_absorption_factor = std::clamp(defaults.air_absorption, 0.f, 10.f);
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

		if (!m.disable_velocity) {
			if (interp_enabled || when_set_velocity != cosm.get_timestamp()) {
				cmd.set_velocity = true;
				cmd.velocity = effective_velocity;

				const auto sq_max = in.settings.max_speed_for_doppler_calculation * in.settings.max_speed_for_doppler_calculation;
				const auto sq_len = cmd.velocity.length_sq();

				if (sq_len > sq_max) {
					cmd.velocity.set_length(in.settings.max_speed_for_doppler_calculation);
				}
			}
		}

		when_set_velocity = cosm.get_timestamp();
	}

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

	if (max_dist == 0.f) {
		max_dist = 1.f;
	}

	const bool is_linear = 
		dist_model == augs::distance_model::LINEAR_DISTANCE
		|| dist_model == augs::distance_model::LINEAR_DISTANCE_CLAMPED
	;

	const bool is_nonlinear = !is_linear && dist_model != augs::distance_model::NONE;

	const auto mult_via_settings = std::clamp([&]() {
		const auto master = in.volume.master;

		if (original.input.modifier.always_direct_listener) {
			if (const auto buf = source.buffer_meta; buf.is_set()) {
				if (buf.computed_length_in_seconds > in.settings.treat_as_music_sounds_longer_than_secs) {
					return master * in.volume.music;
				}
			}
		}

		return master * in.volume.sound_effects;
	}(), 0.f, 1.f);

	const auto flash_mult = in.owner.get_effective_flash_mult();

	float custom_dist_gain_mult = 1.f;

	if (is_linear) {

	}
	else if (is_nonlinear && !is_direct_listener && listening_character) {
		/* Let's just do our custom gain calculation */

		const auto dist = (current_transform.pos - listening_character.get_viewing_transform(in.interp).pos).length();

		if (dist > ref_dist) {
			custom_dist_gain_mult *= 1 - std::clamp(dist - ref_dist, 0.f, max_dist) / max_dist;
		}

		custom_dist_gain_mult *= custom_dist_gain_mult;
	}

	if (flash_mult > 0.f) {
		cmd.lowpass_gainhf = 1.f - flash_mult;
	}

	cmd.proxy_id = source.id;
	cmd.si = cosm.get_si();
	cmd.position = current_transform.pos;
	cmd.gain = std::clamp((1 - flash_mult) * std::clamp(m.gain, 0.f, 1.f) * mult_via_settings * custom_dist_gain_mult, 0.f, 1.f);
	cmd.pitch = m.pitch * in.speed_multiplier;
	cmd.doppler_factor = std::max(0.f, m.doppler_factor);
	cmd.reference_distance = ref_dist;
	cmd.max_distance = max_dist;
	cmd.looping = m.repetitions == -1;
	cmd.model = dist_model;
	cmd.is_direct_listener = is_direct_listener;
	cmd.update(source);

	if (!gain_dependent_lifetime || elapsed_secs != 0.f) {
		in.renderer.push_command(cmd);
	}

	if (!(in.dt == augs::delta::zero)) {
		if (gain_dependent_lifetime) {
			if (elapsed_secs == 0.f) {
				if (custom_dist_gain_mult > in.settings.gain_threshold_for_bullet_trace_sounds) {
					auto& current = in.owner.current_num_silent_traces;

					if (current < in.settings.max_simultaneous_bullet_trace_sounds) {
						elapsed_secs = dt_this_frame;

						in.renderer.push_command(cmd);

						const auto proxy = get_proxy(in);
						proxy.play();

						++current;
					}
					else {
						original.input.modifier.repetitions = 0;
					}
				}
			}
		}

		if (gain_dependent_lifetime) {
			if (elapsed_secs != 0.f) {
				if (custom_dist_gain_mult < in.settings.gain_threshold_for_bullet_trace_sounds) {
					original.input.modifier.repetitions = 0;
				}
			}
		}
	}
}

void sound_system::generic_sound_cache::maybe_play_next(update_properties_input in) {
	if (probably_still_playing()) {
		return;
	}

	if (followup_inputs.empty()) {
		return;
	}

	const auto proxy = get_proxy(in);
	eat_followup();

	proxy.stop();
	elapsed_secs = 0.f;

	if (rebind_buffer(in)) {
		update_properties(in);
		proxy.play();
	}
}

bool sound_system::generic_sound_cache::probably_still_playing() const {
	const auto& input = original.input;
	const auto& m = input.modifier;
	const auto reps = m.repetitions;

	if (reps < 0) {
		return true;
	}

	return elapsed_secs <= source.buffer_meta.computed_length_in_seconds * reps;
}

void sound_system::update_effects_from_messages(const const_logic_step step, const update_properties_input in) {
	{
		const auto& events = step.get_queue<messages::stop_sound_effect>();

		for (auto& e : events) {
			erase_if(short_sounds, [&](generic_sound_cache& c) {	
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
					if (!start_fading(c)) {
						c.stop_and_free(in);
					}
				}
				else {
					c.stop_and_free(in);
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
					collision_cooldown_key key;
					key.participants = s;

					if constexpr(std::is_same_v<decltype(dummy), messages::start_sound_effect*>) {
						key.id = e.payload.input.id;
					}
					else {
						if (e.payload.inputs.size() > 0) {
							key.id = e.payload.inputs[0].id;
						}
					}

					if (start.is_missile_impact()) {
						const auto max_occurences = in.settings.missile_impact_occurences_before_cooldown;

						const auto it = damage_sound_cooldowns.try_emplace(key);
						auto& cooldown = (*it.first).second;

						if (cooldown.consecutive_occurences >= max_occurences) {
							continue;
						}

						++cooldown.consecutive_occurences;
						cooldown.max_ms = in.settings.missile_impact_sound_cooldown_duration;
					}
					else {
						const auto it = collision_sound_cooldowns.try_emplace(key);

						auto& cooldown = (*it.first).second;

						++cooldown.consecutive_occurences;
						cooldown.remaining_ms = start.collision_sound_cooldown_duration;

						// LOG_NVPS(cooldown.consecutive_occurences);

						if (cooldown.consecutive_occurences > start.collision_sound_occurences_before_cooldown) {
							// LOG("Skipping");
							continue;
						}
					}
				}
			}

			if (in.settings.max_short_sounds > 0 && !id_pool.full() && short_sounds.size() < short_sounds.max_size()) {
				if (!in.under_short_sound_limit()) {
					if (short_sounds.size() > 0) {
						short_sounds[0].stop_and_free(in);
						short_sounds.erase(short_sounds.begin());
					}
				}

				const auto new_id = id_pool.allocate();

				auto release_id = [&]() {
					id_pool.free(new_id);
				};

				try {
					short_sounds.emplace_back(new_id, e.payload, in);
				}
				catch (const effect_not_found&) {
					release_id();
				}
				catch (const shouldnt_play&) {
					release_id();
				}
			}
		}
	};

	do_events((messages::start_sound_effect*)(nullptr));
	do_events((messages::start_multi_sound_effect*)(nullptr));
}

void sound_system::advance_flash(const const_entity_handle listener, const augs::delta dt) {
	if (listener.dead()) {
		return;
	}

	const auto sentience = listener.template find<invariants::sentience>();

	if (sentience == nullptr) {
		return;
	}

	const auto mult = get_flash_audio_mult(listener);

	auto& last_mult = last_registered_flash_mult;

	if (mult > 0.f) {
		if (mult > last_mult) {
			after_flash_passed_ms += dt.in_milliseconds();

			const auto delay_ms = sentience->flash_effect_delay_ms;

			if (after_flash_passed_ms >= delay_ms) {
				after_flash_passed_ms = 0;
				last_mult = mult;
			}
		}
	}
	else {
		after_flash_passed_ms = 0.f;
	}

	if (mult <= last_mult) {
		last_mult = mult;
	}
}

void sound_system::update_sound_properties(const update_properties_input in) {
	const auto& renderer = in.renderer;
	const auto listening_character = in.get_listener();

	const auto screen_center = in.camera.get_world_screen_center();

	update_listener(
		in.renderer,
		listening_character, 
		in.interp, 
		in.settings,
		screen_center
	);

	silent_trace_cooldown += in.dt.in_milliseconds();

	if (silent_trace_cooldown > 150.f) {
		silent_trace_cooldown = 0.f;

		--current_num_silent_traces;

		if (current_num_silent_traces < 0) {
			current_num_silent_traces = 0;
		}
	}

	const auto& cosm = listening_character.get_cosmos();

	{
		advance_flash(listening_character, in.dt);

		const auto& last_mult = last_registered_flash_mult;

		const auto effect = cosm.get_common_assets().flash_noise_sound;

		if (auto buf = mapped_or_nullptr(in.manager, effect.id)) {
			augs::update_flash_noise cmd;
			cmd.buffer = buf;
			cmd.gain = last_mult * in.volume.get_sound_effects_volume();
			renderer.push_command(cmd);
		}
	}

	cosm.for_each_having<components::gun>(
		[&](const auto gun_entity) {
			const auto id = gun_entity.get_id().to_unversioned();

			if (const auto sound = ::calc_firearm_engine_sound(gun_entity)) {
				if (auto* const existing = mapped_or_nullptr(firearm_engine_caches, id)) {
					existing->cache.original = *sound;

					if (!existing->cache.rebind_buffer(in)) {
						fade_and_erase(in, firearm_engine_caches, id);
					}
				}
				else {
					if (!id_pool.full()) {
						const auto new_id = id_pool.allocate();

						auto release_id = [&]() {
							id_pool.free(new_id);
						};

						try {
							firearm_engine_caches.try_emplace(id, continuous_sound_cache { { new_id, *sound, in }, { gun_entity.get_name() } } );
						}
						catch (const effect_not_found&) {
							release_id();
						}
						catch (const shouldnt_play&) {
							release_id();
						}
					}
				}
			}
			else {
				fade_and_erase(in, firearm_engine_caches, id);
			}
		}
	);

	cosm.for_each_having<invariants::continuous_sound>(
		[&](const auto sound_entity) {
			const auto id = sound_entity.get_id().to_unversioned();
			const auto& continuous_sound = sound_entity.template get<invariants::continuous_sound>();

			if (!continuous_sound.effect.id.is_set()) {
				fade_and_erase(in, continuous_sound_caches, id);
				return;
			}

			packaged_sound_effect sound;

			sound.start = sound_effect_start_input::at_entity(sound_entity);
			sound.start.set_listener(sound_entity.get_owning_transfer_capability());

			sound.input = continuous_sound.effect;

			if (const auto item = sound_entity.template find<components::item>()) {
				if (const auto slot = sound_entity.get_current_slot(); slot.alive()) {
					if (!slot.is_hand_slot()) {
						fade_and_erase(in, continuous_sound_caches, id);
						return;
					}

					if (sound_entity.find_colliders_connection() == nullptr) {
						fade_and_erase(in, continuous_sound_caches, id);
						return;
					}
				}
			}

			if (auto* const existing = mapped_or_nullptr(continuous_sound_caches, id)) {
				existing->cache.original = sound;

				if (!existing->cache.rebind_buffer(in)) {
					fade_and_erase(in, continuous_sound_caches, id);
				}
			}
			else {
				if (!id_pool.full()) {
					const auto new_id = id_pool.allocate();

					auto release_id = [&]() {
						id_pool.free(new_id);
					};

					try {
						continuous_sound_caches.try_emplace(id, continuous_sound_cache { { new_id, sound, in }, { sound_entity.get_name() } } );
					}
					catch (const effect_not_found&) {
						release_id();
					}
					catch (const shouldnt_play&) {
						release_id();
					}
				}
			}
		}
	);

	auto update_facade = [&](auto& cache) {
		cache.update_properties(in);
		cache.maybe_play_next(in);

		const bool still_playing_or_has_followup = cache.probably_still_playing() || cache.followup_inputs.size() > 0;
		return !still_playing_or_has_followup;
	};

	auto fade_or_stop = [&](auto& c) {
		if (!start_fading(c)) {
			c.stop_and_free(in);
		}
	};

	erase_if(firearm_engine_caches, [&](auto& it) {
		const auto& recorded = it.second.recorded;
		const auto handle = cosm[it.first];

		auto& cache = it.second.cache;

		if (handle.dead()) {
			fade_or_stop(cache);
			return true;
		}

		if (recorded.name != handle.get_name()) {
			fade_or_stop(cache);
			return true;
		}

		const bool result = update_facade(cache);

		if (result) {
			cache.stop_and_free(in);
		}

		return result;
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

	auto reseek_if_diverged = [&](const auto& subject, const auto& cache) {
		const auto& source = cache.source;
		const auto& m = cache.original.input.modifier;

		if (const auto buf = source.buffer_meta; buf.is_set()) {
			const auto secs = buf.computed_length_in_seconds;

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

				augs::reseek_to_sync_if_needed cmd;
				cmd.proxy_id = source.id;
				cmd.expected_secs = expected_secs;
				cmd.max_divergence = max_divergence;
				
				renderer.push_command(cmd);
			}
		}
	};

	erase_if(continuous_sound_caches, [&](auto& it) {
		const auto& recorded = it.second.recorded;
		const auto handle = cosm[it.first];

		auto& cache = it.second.cache;

		if (handle.dead()) {
			fade_or_stop(cache);
			return true;
		}

		if (!can_have_continuous_sound(handle)) {
			fade_or_stop(cache);
			return true;
		}

		if (recorded.name != handle.get_name()) {
			fade_or_stop(cache);
			return true;
		}

		const auto result = update_facade(cache);

		if (!result) {
			reseek_if_diverged(cosm[it.first], cache);
		}

		if (result) {
			cache.stop_and_free(in);
		}

		return result;
	});

	erase_if(short_sounds, [&](generic_sound_cache& cache) {
		const auto& start = cache.original.start;

		const auto logical_subject = cosm[start.positioning.target];

		auto erase_by_fade = [&]() {
			fade_or_stop(cache);
			return true;
		};

		if (logical_subject.dead()) {
			if (start.clear_when_target_entity_deleted) {
				return erase_by_fade();
			}
		}
		else {
			if (start.clear_when_target_alive) {
				if (::sentient_and_alive(logical_subject)) {
					return erase_by_fade();
				}
			}

			if (start.clear_when_target_conscious) {
				if (::sentient_and_conscious(logical_subject)) {
					return erase_by_fade();
				}
			}
		}

		const bool result = update_facade(cache);

		if (result) {
			cache.stop_and_free(in);
		}

		return result;
	});
}

void sound_system::fade_sources(
	const augs::audio_renderer& renderer,
	const augs::delta dt
) {
	erase_if (collision_sound_cooldowns, [&](auto& it) {
		auto& c = it.second;

		c.remaining_ms -= dt.in_milliseconds();

		if (c.remaining_ms <= 0.f) {
			return true;
		}

		return false;
	});

	erase_if (damage_sound_cooldowns, [&](auto& it) {
		auto& c = it.second;

		c.current_ms += dt.in_milliseconds();

		if (c.current_ms > c.max_ms) {
			c.current_ms = 0.f;
			--c.consecutive_occurences;

			if (0 == c.consecutive_occurences) {
				return true;
			}
		}

		return false;
	});

	erase_if(fading_sources, [this, dt, &renderer](fading_source& f) {
		const auto proxy = augs::sound_source_proxy { renderer, f.source };

		const auto new_gain = proxy.get_gain() - dt.in_seconds() * f.fade_per_sec;
		const auto new_pitch = proxy.get_pitch() - dt.in_seconds() * f.fade_per_sec;

		if (new_pitch > 0.1f) {
			proxy.set_pitch(new_pitch);
		}

		if (new_gain > 0.f) {
			proxy.set_gain(new_gain);
			return false;
		}

		proxy.stop();
		id_pool.free(f.source.id);
		return true;
	});
}
