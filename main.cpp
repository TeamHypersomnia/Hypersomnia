#pragma once
#include "game_framework/game_framework.h"
#include "game/hypersomnia_overworld.h"

using namespace augs;

#include "game_framework/scene_builders/scene_builders.h"

int main(int argc, char** argv) {
	framework::init();
	//framework::run_tests();

	hypersomnia_overworld overworld;
	overworld.initialize();
	overworld.configure_scripting();
	overworld.call_window_script("config.lua");

	scene_builders::testbed(overworld.game_world);

	overworld.simulate();

	framework::deinit();
	return 0;
}