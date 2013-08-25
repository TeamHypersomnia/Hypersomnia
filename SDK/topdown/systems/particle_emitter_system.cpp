#include "particle_emitter_system.h"
#include "entity_system/entity.h"

particle_emitter_system::particle_emitter_system() : generator(device()) {}

void particle_emitter_system::spawn_particle(
	components::particle_group& group, const vec2<>& position, float rotation, const components::particle_emitter::emission& emission) {
		auto new_particle = emission.particle_templates[randval(0u, emission.particle_templates.size()-1)];
		new_particle.vel = vec2<>::from_angle(
			randval(rotation - emission.spread_radians, rotation + emission.spread_radians)) *
			randval(emission.velocity_min, emission.velocity_max);
		
		new_particle.pos = position + emission.offset;
		new_particle.lifetime_ms = 0.f;
		new_particle.face.size *= randval(emission.size_multiplier_min, emission.size_multiplier_max);
		new_particle.rotation = randval(rotation - emission.initial_rotation_variation, rotation + emission.initial_rotation_variation);
		new_particle.rotation_speed = randval(emission.angular_velocity_min, emission.angular_velocity_max);

		group.particles.push_back(new_particle);
}

void particle_emitter_system::process_entities(world& owner) {
	using namespace components;
	using namespace messages;

	double delta = timer.extract<std::chrono::milliseconds>();

	auto events = owner.get_message_queue<particle_burst_message>();

	for (auto it : events) {
		auto* emitter = it.subject->find<particle_emitter>();
		if (emitter == nullptr) continue;

		auto emissions = emitter->available_emissions->find(it.type);

		if (emissions == emitter->available_emissions->end())
			continue;

		for (auto& emission : (*emissions).second) {
			float target_rotation = it.rotation + emission.angular_offset;

			if (emission.type == particle_emitter::emission::type::BURST) {
				int burst_amount = randval(emission.particles_per_burst_min, emission.particles_per_burst_max);
				
				for (int i = 0; i < burst_amount; ++i)
					spawn_particle(emission.target_particle_group->get<components::particle_group>(), it.pos, target_rotation, emission);
			}

			else if (emission.type == particle_emitter::emission::type::STREAM) {
				particle_emitter::stream new_stream(&emission);
				new_stream.lifetime_ms = 0.f;
				new_stream.max_lifetime_ms = randval(emission.stream_duration_ms_min, emission.stream_duration_ms_max);
				new_stream.particles_to_spawn = 0.f;
				new_stream.pos = it.pos;
				new_stream.rotation = target_rotation;

				emitter->current_streams.push_back(new_stream);
			}
		}
	}

	for (auto it : targets) {
		auto& emitter = it->get<particle_emitter>();

		for (auto stream = emitter.current_streams.begin(); stream != emitter.current_streams.end(); ++stream) {
			delta = std::min(delta, static_cast<double>((*stream).max_lifetime_ms - (*stream).lifetime_ms));

			(*stream).lifetime_ms += delta;
			(*stream).lifetime_ms = std::min((*stream).lifetime_ms, (*stream).max_lifetime_ms);

			(*stream).particles_to_spawn += randval((*stream).info->particles_per_sec_min, (*stream).info->particles_per_sec_max) * (delta / 1000.0);

			while ((*stream).particles_to_spawn >= 1.f) {
				spawn_particle((*stream).info->target_particle_group->get<components::particle_group>(), (*stream).pos, (*stream).rotation, *(*stream).info);
				(*stream).particles_to_spawn -= 1.f;
			}
		}

		emitter.current_streams.erase(std::remove_if(emitter.current_streams.begin(), emitter.current_streams.end(),
			[](const particle_emitter::stream& a) { return a.lifetime_ms >= a.max_lifetime_ms;  }
		), emitter.current_streams.end());
	}
}
