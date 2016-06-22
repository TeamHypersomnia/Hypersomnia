#include "multiverse.h"

#include <signal.h>

#include "window_framework/platform_utils.h"
#include "scripting/script.h"

#include "game/bindings/bind_game_and_augs.h"
#include "game/messages/camera_render_request_message.h"

#include "game/systems/input_system.h"
#include "game/systems/render_system.h"
#include "game/stateful_systems/gui_system.h"
#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"

#include <luabind/luabind.hpp>

#include "log.h"

using namespace std;
using namespace augs;

void SignalHandler(int signal) { 
	window::disable_cursor_clipping();
	throw "Access violation!"; 
}

multiverse::multiverse() {
	main_cosmos.reserve_storage_for_aggregates(50000);
}

void multiverse::configure_scripting() {
	bind_game_and_augs(lua);
	signal(SIGSEGV, SignalHandler);
}

void multiverse::call_window_script(std::string filename) {
	lua.global_ptr("global_gl_window", &game_window);

	try {
		if (!lua.dofile(filename))
			lua.debug_response();
	}
	catch (char* e) {
		LOG("Exception thrown! %x", e);
		lua.debug_response();
	}
	catch (...) {
		LOG("Exception thrown!");
		lua.debug_response();
	}

	game_window.gl.initialize();
}

void multiverse::load_resources() {
	resource_setups::load_standard_atlas();
	resource_setups::load_standard_particle_effects();
	resource_setups::load_standard_behaviour_trees();

	resource_manager.create(assets::shader_id::DEFAULT_VERTEX, L"hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
	resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, L"hypersomnia/shaders/default.fsh", augs::graphics::shader::type::FRAGMENT);
	resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

	resource_manager.create(assets::shader_id::DEFAULT_HIGHLIGHT_VERTEX, L"hypersomnia/shaders/default_highlight.vsh", augs::graphics::shader::type::VERTEX);
	resource_manager.create(assets::shader_id::DEFAULT_HIGHLIGHT_FRAGMENT, L"hypersomnia/shaders/default_highlight.fsh", augs::graphics::shader::type::FRAGMENT);
	resource_manager.create(assets::program_id::DEFAULT_HIGHLIGHT, assets::shader_id::DEFAULT_HIGHLIGHT_VERTEX, assets::shader_id::DEFAULT_HIGHLIGHT_FRAGMENT);

	resource_manager.create(assets::shader_id::CIRCULAR_BARS_VERTEX, L"hypersomnia/shaders/circular_bars.vsh", augs::graphics::shader::type::VERTEX);
	resource_manager.create(assets::shader_id::CIRCULAR_BARS_FRAGMENT, L"hypersomnia/shaders/circular_bars.fsh", augs::graphics::shader::type::FRAGMENT);
	resource_manager.create(assets::program_id::CIRCULAR_BARS, assets::shader_id::CIRCULAR_BARS_VERTEX, assets::shader_id::CIRCULAR_BARS_FRAGMENT);
}

void multiverse::build_scene() {
	resource_manager.destroy_everything();
	current_scene_manager->load_resources();
	
	main_cosmos.delete_all_entities();
	current_scene_manager->populate_world_with_entities(main_cosmos);
	
	clear_window_inputs_once = true;
}

#define RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC 0

float stepping_speed = 1.f;
void multiverse::main_game_loop() {
	bool quit_flag = false;

	while (!quit_flag) {
		main_cosmos.fps_counter.new_measurement();

		main_step.messages.get_queue<messages::raw_window_input_message>().clear();

		auto raw_window_inputs = game_window.poll_events();

		if (clear_window_inputs_once) {
			raw_window_inputs.clear();
			clear_window_inputs_once = false;
		}

		for (auto& raw_input : raw_window_inputs) {
			if (raw_input.key_event == window::event::PRESSED) {
				if (raw_input.key == window::event::keys::ESC) {
					quit_flag = true;
					break;
				}
				if (raw_input.key == window::event::keys::_1) {
					configure_stepping(60, 500000);
				}
				if (raw_input.key == window::event::keys::_2) {
					configure_stepping(128, 500000);
				}
				if (raw_input.key == window::event::keys::_3) {
					configure_stepping(400, 500000);
				}
				if (raw_input.key == window::event::keys::_4) {
					stepping_speed = 0.1f;
				}
				if (raw_input.key == window::event::keys::_5) {
					stepping_speed = 1.f;
				}
				if (raw_input.key == window::event::keys::_6) {
					stepping_speed = 6.f;
				}
				if (raw_input.key == window::event::keys::F4) {
					LOG_COLOR(console_color::YELLOW, "Separator");
				}
			}

			messages::raw_window_input_message msg;
			msg.raw_window_input = raw_input;

			if(!main_cosmos.systems.get<input_system>().is_replaying())
				main_cosmos.post_message(msg);
		}

		delta_timer.set_stepping_speed_multiplier(stepping_speed);

#if RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC
		auto steps_to_perform = delta_timer.count_logic_steps_to_perform();

		while (steps_to_perform--) {
#endif

		assign_frame_time_to_delta_for_drawing_time_systems();

		enable_drawing_time_random_generator();
		main_cosmos.call_drawing_time_systems();

		consume_camera_render_requests();

		main_cosmos.restore_transforms_after_drawing();

		restore_fixed_delta();
		enable_deterministic_random_generator();

#if !RENDERING_STEPS_DETERMINISTICALLY_LIKE_LOGIC
		auto steps_to_perform = delta_timer.count_logic_steps_to_perform();

		while (steps_to_perform--) {
#endif
			renderer::get_current().clear_logic_lines();

			main_cosmos.perform_logic_step();
			current_scene_manager->perform_logic_step(main_cosmos);
		}

		main_cosmos.fps_counter.end_measurement();
	}
}

void multiverse::consume_camera_render_requests() {
	auto& requests = main_step.messages.get_queue<messages::camera_render_request_message>();
	auto& target = renderer::get_current();

	target.clear_current_fbo();

	for (auto& r : requests) {
		target.set_viewport(r.state.viewport);
		current_scene_manager->execute_drawcalls_for_camera(r);
	}

	current_scene_manager->drawcalls_after_all_cameras(main_cosmos);
	
	main_cosmos.triangles.measure(target.triangles_drawn_total);
	target.triangles_drawn_total = 0;

	game_window.swap_buffers();
}
