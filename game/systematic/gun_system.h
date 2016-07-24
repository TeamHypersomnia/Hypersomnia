#pragma once

class physics_system;
class cosmos;
class fixed_step;

class gun_system {
public:
	
	void consume_gun_intents(fixed_step& step);
	void launch_shots_due_to_pressed_triggers(fixed_step& step);
};