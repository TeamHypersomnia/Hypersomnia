#pragma once
#include "game/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_overworld.h"

using namespace augs;

#include "game/scene_builders/testbed.h"

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest();

	game_overworld hypersomnia_overworld;
	hypersomnia_overworld.configure_scripting();
	hypersomnia_overworld.call_window_script("config.lua");

	hypersomnia_overworld.set_scene_builder(std::unique_ptr<scene_builder>(new scene_builders::testbed));
	hypersomnia_overworld.build_scene();

	hypersomnia_overworld.main_game_loop();

	augs::global_libraries::deinit();
	return 0;
}