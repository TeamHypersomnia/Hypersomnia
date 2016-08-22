#pragma once
#include "augs/global_libraries.h"
#include "setups.h"

#include "game/game_window.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest(argc, argv);

	game_window window;
	window.call_window_script("config.lua");

	resource_manager.destroy_everything();
	resource_setups::load_standard_everything();

	client_setup setup;
	setup.process(window);

	augs::global_libraries::deinit();
	return 0;
}