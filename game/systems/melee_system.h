#pragma once
#include "entity_system/processing_system.h"
#include "game/components/melee_component.h"
#include "game/components/damage_component.h"

using namespace augs;

class melee_system : public processing_system_templated<components::melee, components::damage> {
public:
	using processing_system_templated::processing_system_templated;

	void consume_melee_intents();
	void initiate_and_update_moves();
};