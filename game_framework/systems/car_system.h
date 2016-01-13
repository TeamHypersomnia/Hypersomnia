#pragma once
#include "entity_system/processing_system.h"
#include "../components/car_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

class car_system : public augs::processing_system_templated<components::car, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void set_steering_flags_from_intents();

	void apply_movement_forces();
};