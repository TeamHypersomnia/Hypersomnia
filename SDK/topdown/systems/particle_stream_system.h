#pragma once
#include "utility/timer.h"

#include "entity_system/processing_system.h"

#include "../components/particle_stream_component.h"

using namespace augmentations;
using namespace entity_system;

class particle_stream_system : public processing_system_templated<components::particle_stream, components::transform> {
	util::timer timer;
public:
	void process_entities(world&);
};