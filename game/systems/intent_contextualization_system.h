#pragma once
#include "entity_system/processing_system.h"

using namespace augs;

class intent_contextualization_system : public event_only_system {
public:
	using event_only_system::event_only_system;

	void contextualize_movement_intents();
	void contextualize_use_button_intents();
	void contextualize_crosshair_action_intents();
};