#pragma once
#include <random>
#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

using namespace augs;

class physics_system;

class gun_system : public processing_system_templated<components::transform, components::gun> {
public:
	using processing_system_templated::processing_system_templated;
	
	void consume_gun_intents();
	void launch_shots_due_to_pressed_triggers();
};