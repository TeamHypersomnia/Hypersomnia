#include "sound_system.h"

#include "augs/audio/sound_buffer.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/systems_audiovisual/interpolation_system.h"
#include "game/resources/manager.h"

void sound_system::resample_state_for_audiovisuals(const cosmos& new_cosmos) {
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
		per_entity_cache.erase(it);
	}
}

void sound_system::initialize_sound_sources(const size_t num_max_sources) {
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
	queried_size.set(10000.f, 10000.f);

	const float pixels_to_metersf = PIXELS_TO_METERSf;

	const auto targets =
		cosmos.get(processing_subjects::WITH_SOUND_EXISTENCE);

		//cosmos[cosmos.systems_temporary.get<dynamic_tree_system>()
		//.determine_visible_entities_from_camera(cone, components::dynamic_tree_node::tree_type::SOUND_EXISTENCES)];

	const auto subject = cosmos[listening_character];

	const auto listener_pos = subject.viewing_transform(sys).pos;

	augs::set_listener_position(listener_pos);
	augs::set_listener_velocity(subject.get_effective_velocity());
	augs::set_listener_orientation({ 0.f, -1.f, 0.f, 0.f, 0.f, -1.f });

	for (const auto it : targets) {
		auto& cache = get_cache(it);
		const auto& existence = it.get<components::sound_existence>();
		auto& source = cache.source;

		const auto& buffer = get_resource_manager().find(existence.input.effect)->get_variation(existence.input.variation_number);

		const auto& requested_buf = existence.input.direct_listener == listening_character ? buffer.request_stereo() : buffer.request_mono();

		if (
			cache.recorded_component.time_of_birth != existence.time_of_birth
			|| cache.recorded_component.input.effect != existence.input.effect
			|| &requested_buf != source.get_bound_buffer()
		) {
			source.stop();

			if (listening_character == existence.input.direct_listener) {
				source.bind_buffer(buffer.request_stereo());
				source.set_direct_channels(true);
			}
			else {
				source.bind_buffer(buffer.request_mono());
				source.set_direct_channels(false);
			}

			source.play();
			source.set_max_distance(existence.input.modifier.max_distance);
			source.set_reference_distance(existence.input.modifier.reference_distance);
			source.set_looping(existence.input.modifier.repetitions == -1);

			cache.recorded_component = existence;
		}

		const auto source_pos = it.viewing_transform(sys).pos;
		const auto dist_from_listener = (listener_pos - source_pos).length();
		const float absorption = std::min(10.f, pow(std::max(0.f, dist_from_listener - 2220.f)/520.f, 2));

		source.set_air_absorption_factor(absorption);
		source.set_pitch(existence.input.modifier.pitch);
		source.set_gain(existence.input.modifier.gain * master_gain);
		source.set_position(source_pos);
		source.set_velocity(it.get_effective_velocity());
	}
}
