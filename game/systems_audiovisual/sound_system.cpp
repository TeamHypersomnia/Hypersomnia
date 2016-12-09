#include "sound_system.h"

#include "augs/audio/sound_buffer.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/systems_audiovisual/interpolation_system.h"
#include "game/resources/manager.h"

#include <AL/al.h>
#include <AL/alc.h>

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
	alSpeedOfSound(120.f);
	alDistanceModel(AL_EXPONENT_DISTANCE);
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
	cone.visible_world_area *= 2.5;

	const float pixels_to_metersf = PIXELS_TO_METERSf;

	const auto targets =
		cosmos[cosmos.systems_temporary.get<dynamic_tree_system>()
		.determine_visible_entities_from_camera(cone, components::dynamic_tree_node::tree_type::SOUND_EXISTENCES)];

	for (const auto it : targets) {
		auto& cache = get_cache(it);
		const auto& existence = it.get<components::sound_existence>();
		const auto& source = cache.source;

		if (cache.recorded_component.time_of_birth != existence.time_of_birth
			|| cache.recorded_component.input.effect != existence.input.effect) {
			
			AL_CHECK(alSourcei(source, AL_BUFFER, get_resource_manager().find(existence.input.effect)->get_id()));
			AL_CHECK(alSourcef(source, AL_PITCH, 1));
			AL_CHECK(alSourcef(source, AL_GAIN, 1));
			AL_CHECK(alSourcei(source, AL_LOOPING, AL_FALSE));
			AL_CHECK(alSourcePlay(source));

			cache.recorded_component = existence;
		}

		auto transform = it.viewing_transform(sys);
		transform.pos *= pixels_to_metersf;

		const auto vel = it.get_effective_velocity() * pixels_to_metersf;

		alSource3f(source, AL_POSITION, transform.pos.x, transform.pos.y, 0);
		alSource3f(source, AL_VELOCITY, vel.x, vel.y, 0);
	}

	const auto subject = cosmos[listening_character];
	auto transform = subject.viewing_transform(sys);
	transform.pos *= pixels_to_metersf;
	const auto vel = subject.get_effective_velocity()* pixels_to_metersf;

	const float listener_orientation[] = {
		0.f,//vec2().set_from_degrees(transform.rotation).x,
		-1.f,//vec2().set_from_degrees(transform.rotation).y,
		0.f,

		0.f,
		0.f,
		-1.f
	};

	alListener3f(AL_POSITION, transform.pos.x, transform.pos.y, 0.f);
	alListener3f(AL_VELOCITY, vel.x, vel.y, 0.f);
	alListenerfv(AL_ORIENTATION, listener_orientation);
}
