#pragma once
#include "entity_system/processing_system.h"
#include "game/components/sentience_component.h"
#include "game/messages/health_event.h"

using namespace augs;

class sentience_system : public processing_system_templated<components::sentience> {
	void consume_health_event(messages::health_event);

public:
	using processing_system_templated::processing_system_templated;

	void apply_damage_and_generate_health_events();
	void cooldown_aimpunches();
	void regenerate_values();
};