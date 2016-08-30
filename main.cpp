#pragma once
#include "augs/global_libraries.h"
#include "setups.h"

#include "game/game_window.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include <thread>

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest(argc, argv);
	
	//sizeof(cosmos);
	//sizeof(augs::pool<int>);

	//sizeof(client_setup);
	//sizeof(server_setup);
	//sizeof(storage_for_all_temporary_systems);
	//sizeof(physics_system);
	//sizeof(b2World);
	//sizeof(b2StackAllocator);

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
	case game_window::launch_mode::LOCAL_DETERMINISM_TEST:
	{
		determinism_test_setup setup;
		setup.process(window);
	}
		break;
	case game_window::launch_mode::CLIENT_AND_SERVER:
	{
		server_setup serv_setup;
		
		std::thread server_thread([&window, &serv_setup]() {
			serv_setup.process(window);
		});
		
		serv_setup.wait_for_listen_server();
		
		client_setup setup;
		setup.process(window);
		
		serv_setup.should_quit = true;
		
		server_thread.join();
	}
	break;
	case game_window::launch_mode::TWO_CLIENTS_AND_SERVER:
	{
		two_clients_and_server_setup setup;
		setup.process(window);
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