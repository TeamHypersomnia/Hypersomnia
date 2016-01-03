#pragma once
#include "game_framework/game_framework.h"
#include "game/hypersomnia_overworld.h"

using namespace augs;

#include "game_framework/scene_builders/all_builders.h"

int main(int argc, char** argv) {
	framework::init();
	//framework::run_tests();

	hypersomnia_overworld overworld;
	overworld.configure_scripting();
	overworld.call_window_script("config.lua");

	overworld.set_scene_builder(std::unique_ptr<scene_builder>(new scene_builders::testbed));
	overworld.initialize_scene();

	overworld.simulate();

	framework::deinit();
	return 0;
}