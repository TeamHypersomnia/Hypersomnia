#pragma once
#include "game_framework/bind_game_framework_and_augs.h"
#include "augs/augmentations.h"
#include "game_framework/game_overworld.h"

using namespace augs;

#include "game_framework/scene_builders/testbed.h"

int main(int argc, char** argv) {
	augs::init();
	//augs::run_tests();

	game_overworld hypersomnia_overworld;
	hypersomnia_overworld.configure_scripting();
	hypersomnia_overworld.call_window_script("config.lua");

	hypersomnia_overworld.set_scene_builder(std::unique_ptr<scene_builder>(new scene_builders::testbed));
	hypersomnia_overworld.initialize_scene();

	hypersomnia_overworld.main_game_loop();

	augs::deinit();
	return 0;
}