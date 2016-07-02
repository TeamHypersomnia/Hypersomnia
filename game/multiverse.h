#pragma once
#include "cosmos.h"
#include "game/scene_managers/testbed.h"
#include "game/entropy_player.h"
#include "misc/fixed_delta_timer.h"
#include "misc/variable_delta_timer.h"

class game_window;

class multiverse {
	scene_managers::testbed main_cosmos_manager;

	mutable augs::variable_delta_timer frame_timer;

	float stepping_speed = 1.f;
public:
	multiverse();

	mutable cosmic_profiler frame_profiler;

	entropy_player main_cosmos_player;

	cosmos main_cosmos;
	augs::fixed_delta_timer main_cosmos_timer;

	void control(augs::machine_entropy);
	void simulate();
	void view(game_window&) const;
};