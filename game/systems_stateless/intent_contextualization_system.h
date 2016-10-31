#pragma once

class cosmos;
class logic_step;

class intent_contextualization_system {
public:

	void contextualize_movement_intents(logic_step& step);
	void contextualize_use_button_intents(logic_step& step);
	void contextualize_crosshair_action_intents(logic_step& step);
};