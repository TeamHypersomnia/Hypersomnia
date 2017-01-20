#pragma once
#include "augs/global_libraries.h"
#include "application/setups/menu_setup.h"
#include "application/setups/local_setup.h"
#include "application/setups/determinism_test_setup.h"
#include "application/setups/two_clients_and_server_setup.h"
#include "application/setups/client_setup.h"
#include "application/setups/server_setup.h"
#include "application/setups/director_setup.h"

#include "application/game_window.h"
#include "application/call_config_script.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/bindings/bind_game_and_augs.h"

#include "augs/scripting/lua_state_raii.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

int main(int argc, char** argv) {
	augs::create_directories("logs/");

	augs::global_libraries::init();
	augs::global_libraries::run_googletest(argc, argv);

	augs::lua_state_raii lua;
	bind_game_and_augs(lua);

	config_lua_table cfg;
	
	call_config_script(lua, "config.lua", "config.local.lua");
	cfg.get_values(lua);

	augs::audio_manager::generate_alsoft_ini(cfg.enable_hrtf);
	augs::audio_manager audio;

	game_window window;
	call_window_script(lua, window, "window.lua");

	resource_setups::load_standard_everything();

	const auto mode = cfg.get_launch_mode();
	LOG("Launch mode: %x", static_cast<int>(mode));

	switch (mode) {
	case config_lua_table::launch_type::MAIN_MENU:
	{
		menu_setup setup;
		setup.process(cfg, window);
	}
		break;
	case config_lua_table::launch_type::LOCAL:
	{
		local_setup setup;
		setup.process(cfg, window);
	}
		break;
	case config_lua_table::launch_type::LOCAL_DETERMINISM_TEST:
	{
		determinism_test_setup setup;
		setup.process(cfg, window);
	}
	case config_lua_table::launch_type::DIRECTOR:
	{
		director_setup setup;
		setup.process(cfg, window);
	}
		break;
	case config_lua_table::launch_type::CLIENT_AND_SERVER:
	{
		server_setup serv_setup;
		
		std::thread server_thread([&]() {
			serv_setup.process(cfg, window);
		});
		
		serv_setup.wait_for_listen_server();
		
		client_setup setup;
		setup.process(cfg, window);
		
		serv_setup.should_quit = true;
		
		server_thread.join();
	}
	break;
	case config_lua_table::launch_type::TWO_CLIENTS_AND_SERVER:
	{
		two_clients_and_server_setup setup;
		setup.process(cfg, window);
	}
	break;
	case config_lua_table::launch_type::ONLY_CLIENT:  
	{

		client_setup setup;
		setup.process(cfg, window);
	}
		break;
	case config_lua_table::launch_type::ONLY_SERVER: 
	{
		server_setup setup;
		setup.process(cfg, window);
	}
		break;

	default: ensure(false); break;
	}

	augs::global_libraries::deinit();
	
	global_log::save_complete_log("logs/successful_exit_debug_log.txt");
	return 0;
}