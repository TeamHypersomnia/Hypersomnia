#pragma once
#include "game/components/sentience_component.h"
#include "game/messages/health_event.h"

using namespace augs;

class sentience_system {
	void consume_health_event(messages::health_event);

public:
	void apply_damage_and_generate_health_events();
	void cooldown_aimpunches();
	void regenerate_values();
};