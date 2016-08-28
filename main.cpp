#pragma once
#include "augs/global_libraries.h"
#include "setups.h"

#include "game/game_window.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"

#include <thread>

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest(argc, argv);

	game_window window;
	window.call_window_script("config.lua");

	resource_manager.destroy_everything();
	resource_setups::load_standard_everything();

	auto mode = window.get_launch_mode();
	LOG("Launch mode: %x", int(mode));

	switch (mode) {
	case game_window::launch_mode::LOCAL:
	{
		local_setup setup;
		setup.process(window);
	}
		break;
	case game_window::launch_mode::CLIENT_AND_SERVER:

	{
		std::thread server_thread([&window]() {
			server_setup setup;
			setup.process(window);
		});

		using namespace std::chrono_literals;

		std::this_thread::sleep_for(1000ms);
		client_setup setup;
		setup.process(window);

		server_thread.join();
	}
		break;
	case game_window::launch_mode::ONLY_CLIENT:  
	{

		client_setup setup;
		setup.process(window);
	}
		break;
	case game_window::launch_mode::ONLY_SERVER: 
	{
		server_setup setup;
		setup.process(window);
	}
		break;

	default: ensure(false); break;
	}

	augs::global_libraries::deinit();
	return 0;
}