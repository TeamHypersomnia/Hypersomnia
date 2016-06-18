#pragma once
#include "game/processing_system_with_cosmos_reference.h"
#include "game/components/car_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

class car_system : public augs::processing_system_templated<components::car, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void set_steering_flags_from_intents();

	void apply_movement_forces();
};