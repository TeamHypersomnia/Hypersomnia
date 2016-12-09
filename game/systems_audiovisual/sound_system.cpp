#include "sound_system.h"

#include "augs/audio/sound_buffer.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/systems_audiovisual/interpolation_system.h"
#include "game/resources/manager.h"

#include <AL/al.h>
#include "augs/al_log.h"

void sound_system::resample_state_for_audiovisuals(const cosmos& new_cosmos) {
	std::vector<entity_id> to_erase;

	for (const auto& it : per_entity_cache) {
		if (new_cosmos[it.first].dead()) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		per_entity_cache.erase(it);
	}
}

void sound_system::initialize_sound_sources(const size_t num_max_sources) {
	AL_CHECK(alSpeedOfSound(120.f));
	AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
	//alDopplerVelocity();
	//alDopplerFactor()
}

sound_system::cache& sound_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

const sound_system::cache& sound_system::get_cache(const const_entity_handle id) const {
	return per_entity_cache.at(id.get_id());
}

void sound_system::play_nearby_sound_existences(
	camera_cone cone, 
	const entity_id listening_character, 
	const cosmos& cosmos, 
	const float global_time_seconds,
	interpolation_system& sys
	) 
{
	auto& queried_size = cone.visible_world_area;

	queried_size.set(1920, 1920);
	queried_size *= 3.5f;

	const float artifacts_avoidance_epsilon = 20.f;
	const float audible_radius = queried_size.x / 2.f - artifacts_avoidance_epsilon;

	const float pixels_to_metersf = PIXELS_TO_METERSf;

	const auto targets =
		cosmos[cosmos.systems_temporary.get<dynamic_tree_system>()
		.determine_visible_entities_from_camera(cone, components::dynamic_tree_node::tree_type::SOUND_EXISTENCES)];

	decltype(per_entity_cache) new_caches;

	for (const auto it : targets) {
		auto& cache = get_cache(it);
		const auto& existence = it.get<components::sound_existence>();
		const auto& source = cache.source;

		if (cache.recorded_component.time_of_birth != existence.time_of_birth
			|| cache.recorded_component.input.effect != existence.input.effect) {
			
			source.attach_buffer(*get_resource_manager().find(existence.input.effect));
			source.play();
			source.set_max_distance(audible_radius);

			cache.recorded_component = existence;
		}

		source.set_position(it.viewing_transform(sys).pos);
		source.set_velocity(it.get_effective_velocity());

		new_caches.emplace(it.get_id(), std::move(cache));
	}

	per_entity_cache = std::move(new_caches);

	const auto subject = cosmos[listening_character];

	augs::set_listener_position(subject.viewing_transform(sys).pos);
	augs::set_listener_velocity(subject.get_effective_velocity());
	augs::set_listener_orientation({ 0.f, -1.f, 0.f, 0.f, 0.f, -1.f });
}
