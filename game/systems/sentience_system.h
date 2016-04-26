#pragma once
#include "entity_system/processing_system.h"
#include "game/components/sentience_component.h"

using namespace augs;

class sentience_system : public processing_system_templated<components::sentience> {
public:
	using processing_system_templated::processing_system_templated;

	void apply_damage_and_generate_health_events();
	void cooldown_aimpunches();
};