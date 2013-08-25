#pragma once
#include <random>
#include "utility/timer.h"

#include "entity_system/processing_system.h"

#include "../messages/particle_burst_message.h"
#include "../components/particle_emitter_component.h"

using namespace augmentations;
using namespace entity_system;

class particle_emitter_system : public processing_system_templated<components::particle_emitter> {
	util::timer timer;
	std::random_device device;
	std::mt19937 generator;

	float randval(int min, int max) {
		if (min == max) return min;
		return std::uniform_int_distribution<int>(min, max)(generator);
	}

	float randval(unsigned min, unsigned max) {
		if (min == max) return min;
		return std::uniform_int_distribution<unsigned>(min, max)(generator);
	}

	float randval(float min, float max) {
		if (min == max) return min;
		return std::uniform_real_distribution<float>(min, max)(generator);
	}

	void spawn_particle(components::particle_group&, const vec2<>&, float, const components::particle_emitter::emission&);
public:
	particle_emitter_system();

	void process_entities(world&);
};