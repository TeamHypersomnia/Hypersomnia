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
#include "game/assets/assets_manager.h"

#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/window_framework/platform_utils.h"

#include "augs/misc/script_utils.h"

/*
	The usage of std::make_unique calls in main is to prevent stack overflow
	due to otherwise there possibly being many cosmoi and resources on the stack.
*/

int main(int argc, char** argv) {
	augs::create_directories("generated/logs/");

	augs::global_libraries::init();
	
	auto cfg = config_lua_table("config.lua", "config.local.lua");
	
	if (cfg.debug_run_unit_tests) {
		augs::global_libraries::run_unit_tests(
			argc, 
			argv,
			cfg.debug_log_successful_unit_tests,
			cfg.debug_break_on_unit_test_failure
		);
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
	
#if !ONLY_ONE_GLOBAL_ASSETS_MANAGER
	auto resources = std::make_unique<assets_manager>();
	resources->set_as_current();
#else
	auto* resources = &get_assets_manager();
#endif

	std::thread regeneration_thread([&cfg](){
		load_all_requisite(cfg);
	});

	{
		augs::window::set_cursor_visible(cfg.debug_disable_cursor_clipping);

		xywhi screen_rect = {
			static_cast<int>(cfg.window_pos.x),	
			static_cast<int>(cfg.window_pos.y),	
			static_cast<int>(cfg.resolution.x),	
			static_cast<int>(cfg.resolution.y)	
		};

		if (cfg.fullscreen) {
			const auto display = augs::window::get_display();

			screen_rect.x = 0;
			screen_rect.y = 0;
			screen_rect.w = display.w;
			screen_rect.h = display.h;

			augs::window::set_display(display.w, display.h, cfg.bpp);
		}

		bool enable_window_border = cfg.window_border;

		if (cfg.fullscreen) {
			enable_window_border = false;
		}

		window.window.create(
			screen_rect,
			enable_window_border,
			cfg.window_name,
			cfg.doublebuffer,
			cfg.bpp
		);

		window.window.set_as_current();
	}

	gl.initialize();
	gl.initialize_fbos(window.get_screen_size());

	regeneration_thread.join();
	load_requisite_atlases(*resources, cfg);
	load_requisite_shaders(*resources);

	const auto mode = cfg.get_launch_mode();
	LOG("Launch mode: %x", static_cast<int>(mode));

	ensure
	(
		(
			mode == launch_type::LOCAL
			|| mode == launch_type::MAIN_MENU
			|| mode == launch_type::CHOREOGRAPHIC
			|| mode == launch_type::DIRECTOR
			|| mode == launch_type::LOCAL_DETERMINISM_TEST
		)
		&& "The launch mode you have chosen is currently out of service."
	);
	
	{
		auto session = viewing_session(window.get_screen_size(), cfg);

		switch (mode) {
		case launch_type::MAIN_MENU:
		{
			auto setup = std::make_unique<menu_setup>();
			setup->process(window, session);
		}
			break;
		case launch_type::LOCAL:
		{
			auto setup = std::make_unique<local_setup>();
			setup->process(window, session);
		}
			break;
		case launch_type::LOCAL_DETERMINISM_TEST:
		{
			auto setup = std::make_unique<determinism_test_setup>();
			setup->process(window, session);
		}
		case launch_type::DIRECTOR:
		{
			auto setup = std::make_unique<director_setup>();
			setup->process(window, session);
		}
			break;

		case launch_type::CHOREOGRAPHIC:
		{
			auto setup = std::make_unique<choreographic_setup>();
			setup->process(window, session);
		}
			break;
		case launch_type::CLIENT_AND_SERVER:
		{
			auto serv_setup = std::make_unique<server_setup>();
			
			std::thread server_thread([&]() {
				serv_setup->process(cfg, window);
			});
			
			serv_setup->wait_for_listen_server();
			
			auto setup = std::make_unique<client_setup>();
			setup->process(window, session);
			
			serv_setup->should_quit = true;
			
			server_thread.join();
		}
		break;
		case launch_type::TWO_CLIENTS_AND_SERVER:
		{
			auto serv_setup = std::make_unique<two_clients_and_server_setup>();
			serv_setup->process(cfg, window);
		}
		break;
		case launch_type::ONLY_CLIENT:  
		{
			auto setup = std::make_unique<client_setup>();
			setup->process(window, session);
		}
			break;
		case launch_type::ONLY_SERVER: 
		{
			auto setup = std::make_unique<server_setup>();
			setup->process(cfg, window);
		}
			break;

		default: ensure(false); break;
		}
	}
	
	//	By now, all sound sources from the viewing session are released.

#if ONLY_ONE_GLOBAL_ASSETS_MANAGER
	/*
		We need to manually destroy the global assets manager,
		before the audio manager gets destroyed.
	*/
	resources->destroy_everything();
#endif

	augs::global_libraries::deinit();
	
	global_log::save_complete_log("generated/logs/successful_exit_debug_log.txt");
	return 0;
}