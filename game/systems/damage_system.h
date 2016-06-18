#pragma once
#include "game/processing_system_with_cosmos_reference.h"

#include "game/components/damage_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

using namespace augs;


class damage_system : public processing_system_templated<components::damage, components::transform, components::physics> {
public:
	using processing_system_templated::processing_system_templated;

	void destroy_colliding_bullets_and_send_damage();
	void destroy_outdated_bullets();
};