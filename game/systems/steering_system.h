#pragma once
#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/steering_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"
#include "game/components/visibility_component.h"

using namespace augs;


class physics_system;

class steering_system : public processing_system_templated<components::transform, components::physics, components::steering> {
public:
	using processing_system_templated::processing_system_templated;

	void substep();
	void process_entities();
};