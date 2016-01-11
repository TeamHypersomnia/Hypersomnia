#pragma once
#include "entity_system/processing_system.h"

#include "../components/lookat_component.h"
#include "../components/transform_component.h"

#include "misc/timer.h"

using namespace augs;


class lookat_system : public processing_system_templated<components::transform, components::lookat> {
	augs::timer smooth_timer;

	void resolve_lookat_value(augs::entity_id lookat);
public:
	using processing_system_templated::processing_system_templated;

	void update_physical_motors();
	void update_rotations();
};