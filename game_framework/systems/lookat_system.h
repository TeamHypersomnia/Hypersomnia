#pragma once
#include "entity_system/processing_system.h"

#include "../components/lookat_component.h"
#include "../components/transform_component.h"

#include "misc/timer.h"

using namespace augs;


class lookat_system : public processing_system_templated<components::transform, components::lookat> {
	augs::misc::timer smooth_timer;
public:
	void process_entities(world&);
};