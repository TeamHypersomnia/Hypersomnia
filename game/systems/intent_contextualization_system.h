#pragma once
#include "game/processing_system_with_cosmos_reference.h"

using namespace augs;

class intent_contextualization_system : public processing_system_with_cosmos_reference {
public:
	using processing_system_with_cosmos_reference::processing_system_with_cosmos_reference;

	void contextualize_movement_intents();
	void contextualize_use_button_intents();
	void contextualize_crosshair_action_intents();
};