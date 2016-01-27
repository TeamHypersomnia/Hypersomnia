#pragma once
#include "game_framework/game_framework.h"
#include "game/hypersomnia_overworld.h"

using namespace augs;

#include "game_framework/scene_builders/all_builders.h"

int main(int argc, char** argv) {
	framework::init();
	//framework::run_tests();

	game_overworld hypersomnia_overworld;
	hypersomnia_overworld.configure_scripting();
	hypersomnia_overworld.call_window_script("config.lua");

	hypersomnia_overworld.set_scene_builder(std::unique_ptr<scene_builder>(new scene_builders::testbed));
	hypersomnia_overworld.initialize_scene();

	hypersomnia_overworld.simulate();

	framework::deinit();
	return 0;
}