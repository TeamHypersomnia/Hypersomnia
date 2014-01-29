#pragma once
#include "entity_system/processing_system.h"

#include "../components/lookat_component.h"
#include "../components/transform_component.h"

#include "misc/timer.h"

using namespace augmentations;
using namespace entity_system;

class lookat_system : public processing_system_templated<components::transform, components::lookat> {
	augmentations::misc::timer smooth_timer;
public:
	void process_entities(world&) override;
};