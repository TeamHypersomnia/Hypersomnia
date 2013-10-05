#include "stdafx.h"
#include "particle_emitter_system.h"
#include "entity_system/entity.h"

#include "../resources/particle_emitter_info.h"

#include "../components/render_component.h"
#include "../components/chase_component.h"
#include "../components/particle_group_component.h"

#include "../messages/particle_burst_message.h"

int randval(int min, int max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_int_distribution<int>(min, max)(generator);
}

unsigned randval(unsigned min, unsigned max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_int_distribution<unsigned>(min, max)(generator);
}

float randval(float min, float max) {
	static std::mt19937 generator = std::mt19937(std::random_device()());
	if (min == max) return min;
	return std::uniform_real_distribution<float>(min, max)(generator);
}

unsigned randval(std::pair<unsigned, unsigned> p) {
	return randval(p.first, p.second);
}

float randval(std::pair<float, float> p) {
	return randval(p.first, p.second);
}

void particle_emitter_system::spawn_particle(
	components::particle_group& group, const vec2<>& position, float rotation, const resources::emission& emission) {
		auto new_particle = emission.particle_templates[randval(0u, emission.particle_templates.size()-1)];
		new_particle.vel = vec2<>::from_angle(
			randval(rotation - emission.spread_degrees, rotation + emission.spread_degrees)) *
			randval(emission.velocity);
		
		new_particle.pos = position + emission.offset;
		new_particle.lifetime_ms = 0.f;
		new_particle.face.size *= randval(emission.size_multiplier);
		new_particle.rotation = randval(rotation - emission.initial_rotation_variation, rotation + emission.initial_rotation_variation);
		new_particle.rotation_speed = randval(emission.angular_velocity);
		new_particle.max_lifetime_ms = randval(emission.particle_lifetime_ms);
		

		if (emission.randomize_acceleration) {
			new_particle.acc += vec2<>::from_angle(
				randval(rotation - emission.spread_degrees, rotation + emission.spread_degrees)) *
				randval(emission.acceleration);
		}

		group.particles.push_back(new_particle);
}

void particle_emitter_system::process_entities(world& owner) {
	using namespace components;
	using namespace messages;

	auto events = owner.get_message_queue<particle_burst_message>();

	for (auto it : events) {
		resources::particle_effect* emissions = nullptr;
		
		if (it.set_effect != nullptr) 
			emissions = it.set_effect;
		else {
			if (it.subject) {
				auto* emitter = it.subject->find<particle_emitter>();
				if (emitter) {
					auto emissions_found = emitter->available_particle_effects->get_raw().find(it.type);

					if (emissions_found == emitter->available_particle_effects->get_raw().end()) continue;
					emissions = &(*emissions_found).second;
				} else continue;
			}
		}


		for (auto& emission : *emissions) {
			float target_rotation = it.rotation + emission.angular_offset;

			if (emission.type == resources::emission::type::BURST) {
				int burst_amount = randval(emission.particles_per_burst);
				
				entity& new_burst_entity = owner.create_entity();
				new_burst_entity.add(components::particle_group());
				new_burst_entity.add(components::transform());
				
				components::render new_render = emission.particle_render_template;
				new_render.set_renderable(&new_burst_entity.get<components::particle_group>());
				new_burst_entity.add(new_render);

				for (int i = 0; i < burst_amount; ++i)
					spawn_particle(new_burst_entity.get<components::particle_group>(), it.pos, target_rotation, emission);
			}

			else if (emission.type == resources::emission::type::STREAM) {
				components::particle_group new_stream;
				new_stream.stream_info = &emission;
				new_stream.stream_lifetime_ms = 0.f;
				new_stream.stream_max_lifetime_ms = randval(emission.stream_duration_ms);
				new_stream.stream_particles_to_spawn = 0.f;

				entity& new_stream_entity = owner.create_entity();
				new_stream_entity.add(new_stream);
				new_stream_entity.add(components::transform(it.pos, target_rotation));

				components::render new_render = emission.particle_render_template;
				new_render.set_renderable(&new_stream_entity.get<components::particle_group>());
				new_stream_entity.add(new_render);

				if (it.subject) {
					components::chase chase(it.subject);
					auto& subject_transform = it.subject->get<components::transform>().current;
					chase.type = components::chase::chase_type::ORBIT;
					chase.rotation_offset = it.rotation - subject_transform.rotation;
					chase.rotation_orbit_offset = (it.pos - subject_transform.pos);

					new_stream_entity.add(chase);
				}
			}
		}
	}
}
