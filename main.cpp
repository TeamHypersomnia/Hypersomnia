#pragma once
#include "game_framework/game_framework.h"
#include "game/hypersomnia_overworld.h"

using namespace augs;

int main(int argc, char** argv) {
	framework::init();
	//framework::run_tests();

	hypersomnia_overworld overworld;
	overworld.initialize();
	overworld.configure_scripting();
	overworld.call_window_script("config.lua");
	overworld.simulate();

	framework::deinit();
	return 0;
}