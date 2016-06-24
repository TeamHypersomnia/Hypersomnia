#pragma once

class cosmos;
class fixed_step;

class intent_contextualization_system {
public:

	void contextualize_movement_intents(fixed_step& step);
	void contextualize_use_button_intents(fixed_step& step);
	void contextualize_crosshair_action_intents(fixed_step& step);
};