#pragma once

class cosmos;
class step_state;

class intent_contextualization_system {
public:

	void contextualize_movement_intents(cosmos& cosmos, step_state& step);
	void contextualize_use_button_intents(cosmos& cosmos, step_state& step);
	void contextualize_crosshair_action_intents(cosmos& cosmos, step_state& step);
};