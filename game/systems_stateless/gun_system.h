#pragma once

class physics_system;
class cosmos;
class logic_step;

class gun_system {
public:
	
	void consume_gun_intents(logic_step& step);
	void launch_shots_due_to_pressed_triggers(logic_step& step);
};