#pragma once
#include "cosmos.h"
#include "game/scene_managers/testbed.h"
#include "game/scene_managers/one_entity.h"
#include "game/transcendental/entropy_player.h"
#include "augs/misc/fixed_delta_timer.h"
#include "augs/misc/variable_delta_timer.h"

class game_window;

class multiverse {
	scene_managers::testbed main_cosmos_manager;
public:
	mutable augs::variable_delta_timer frame_timer;
	mutable augs::measurements fps_profiler = augs::measurements(L"Frame");
	mutable augs::measurements triangles = augs::measurements(L"Triangles", false);

	float stepping_speed = 1.f;

	bool show_profile_details = true;

	void print_summary(augs::renderer&) const;
	std::wstring summary(bool detailed) const;
	std::string recording_filename = "recorded.inputs";
	std::string save_filename = "save.dat";
	std::string saves_folder = "saves/";
	std::string sessions_folder = "sessions/";


	entropy_player main_cosmos_player;

	bool try_to_load_save();
	bool try_to_load_or_save_new_session();

	multiverse();
	
	void populate_cosmoi();

	void save_cosmos_to_file(std::string);
	void load_cosmos_from_file(std::string);

	cosmos main_cosmos;
	augs::fixed_delta_timer main_cosmos_timer;

	void control(augs::machine_entropy);
	void simulate();
	void view(game_window&) const;
};