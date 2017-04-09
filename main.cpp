#pragma once
#include "augs/global_libraries.h"
#include "application/setups/menu_setup.h"
#include "application/setups/local_setup.h"
#include "application/setups/determinism_test_setup.h"
#include "application/setups/two_clients_and_server_setup.h"
#include "application/setups/client_setup.h"
#include "application/setups/server_setup.h"
#include "application/setups/director_setup.h"
#include "application/setups/choreographic_setup.h"

#include "application/game_window.h"
#include "application/call_config_script.h"
#include "game/assets/assets_manager.h"

#include "game/resource_setups/all.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/bindings/bind_game_and_augs.h"

#include "augs/scripting/lua_state_raii.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

/*
	The usage of std::make_unique calls in main is to prevent stack overflow
	due to otherwise there possibly being many cosmoi and resources on the stack.
*/

int main(int argc, char** argv) {
	augs::create_directories("generated/logs/");

	augs::global_libraries::init();

	augs::lua_state_raii lua;
	bind_game_and_augs(lua);

	config_lua_table cfg;
	
	call_config_script(lua, "config.lua", "config.local.lua");
	cfg.get_values(lua);

	if (cfg.debug_run_unit_tests) {
		augs::global_libraries::run_googletest(argc, argv);
	}

	augs::audio_manager::generate_alsoft_ini(
		cfg.enable_hrtf,
		cfg.max_number_of_sound_sources
	);

	augs::audio_manager audio(cfg.audio_output_device);

	game_window window;
	
	augs::renderer gl(
		window.window,
		cfg.debug
	);

	gl.set_as_current();
	
	auto resources = std::make_unique<assets_manager>();
	resources->set_as_current();

	call_window_script(lua, window, "window.lua");

	gl.initialize();
	gl.initialize_fbos(window.get_screen_size());

	load_standard_everything(cfg);

	const auto mode = cfg.get_launch_mode();
	LOG("Launch mode: %x", static_cast<int>(mode));

	ensure
	(
		(
			mode == config_lua_table::launch_type::LOCAL
			|| mode == config_lua_table::launch_type::CHOREOGRAPHIC
			|| mode == config_lua_table::launch_type::DIRECTOR
			|| mode == config_lua_table::launch_type::LOCAL_DETERMINISM_TEST
		)
		&& "The launch mode you have chosen is currently out of service."
	);

	auto session_ptr = std::make_unique<viewing_session>();
	auto& session = *session_ptr;

	session.initialize(window, cfg);

	switch (mode) {
	case config_lua_table::launch_type::MAIN_MENU:
	{
		auto setup = std::make_unique<menu_setup>();
		setup->process(cfg, window, session);
	}
		break;
	case config_lua_table::launch_type::LOCAL:
	{
		auto setup = std::make_unique<local_setup>();
		setup->process(cfg, window, session);
	}
		break;
	case config_lua_table::launch_type::LOCAL_DETERMINISM_TEST:
	{
		auto setup = std::make_unique<determinism_test_setup>();
		setup->process(cfg, window, session);
	}
	case config_lua_table::launch_type::DIRECTOR:
	{
		auto setup = std::make_unique<director_setup>();
		setup->process(cfg, window, session);
	}
		break;

	case config_lua_table::launch_type::CHOREOGRAPHIC:
	{
		auto setup = std::make_unique<choreographic_setup>();
		setup->process(cfg, window, session);
	}
		break;
	case config_lua_table::launch_type::CLIENT_AND_SERVER:
	{
		auto serv_setup = std::make_unique<server_setup>();
		
		std::thread server_thread([&]() {
			serv_setup->process(cfg, window);
		});
		
		serv_setup->wait_for_listen_server();
		
		auto setup = std::make_unique<client_setup>();
		setup->process(cfg, window, session);
		
		serv_setup->should_quit = true;
		
		server_thread.join();
	}
	break;
	case config_lua_table::launch_type::TWO_CLIENTS_AND_SERVER:
	{
		auto serv_setup = std::make_unique<two_clients_and_server_setup>();
		serv_setup->process(cfg, window);
	}
	break;
	case config_lua_table::launch_type::ONLY_CLIENT:  
	{
		auto setup = std::make_unique<client_setup>();
		setup->process(cfg, window, session);
	}
		break;
	case config_lua_table::launch_type::ONLY_SERVER: 
	{
		auto setup = std::make_unique<server_setup>();
		setup->process(cfg, window);
	}
		break;

	default: ensure(false); break;
	}

	augs::global_libraries::deinit();
	
	global_log::save_complete_log("generated/logs/successful_exit_debug_log.txt");
	return 0;
}