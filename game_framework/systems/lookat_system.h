#pragma once
#include "entity_system/processing_system.h"

#include "../components/lookat_component.h"
#include "../components/transform_component.h"

#include "misc/timer.h"

using namespace augs;


class lookat_system : public processing_system_templated<components::transform, components::lookat> {
	augs::timer smooth_timer;
public:
	using processing_system_templated::processing_system_templated;

	void process_entities();
};