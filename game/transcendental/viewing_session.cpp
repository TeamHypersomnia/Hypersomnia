#include "viewing_session.h"
#include "application/game_window.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/scene_managers/rendering_scripts/all.h"
#include "augs/misc/machine_entropy.h"

#include "augs/network/network_client.h"

std::wstring viewing_session::summary() const {
	return 
		fps_profiler.summary()
		+ frame_profiler.summary()
		+ triangles.summary()
		+ local_entropy_profiler.summary()
		+ remote_entropy_profiler.summary()
		+ sending_commands_profiler.summary()
		+ unpack_steps_profiler.summary()
		;
}

void viewing_session::visual_response_to_game_events(const logic_step& step) {
	hud.acquire_game_events(step);
}

void viewing_session::control(const augs::machine_entropy& entropy) {
	for (const auto& raw_input : entropy.local) {
		if (raw_input.key == augs::window::event::keys::key::DASH) {
			show_profile_details = !show_profile_details;
		}
	}
}

void viewing_session::view(const cosmos& cosmos,
	const entity_id viewed_character,
	game_window& window,
	const augs::variable_delta& dt,
	const augs::network::client& details,
	const bool clear_current_and_swap_buffers
) {
	using namespace augs::gui::text;
	
	auto custom_log = multiply_alpha(simple_bbcode(typesafe_sprintf("[color=cyan]Transmission details:[/color]\n%x", details.format_transmission_details()), style(assets::font_id::GUI_FONT, white)), 150.f / 255);;

	view(cosmos, viewed_character, window, dt, custom_log, clear_current_and_swap_buffers);
}

void viewing_session::view(const cosmos& cosmos,
	const entity_id viewed_character,
	game_window& window,
	const augs::variable_delta& dt,
	const augs::gui::text::fstr& custom_log,
	const bool clear_current_and_swap_buffers
	) {
	frame_profiler.new_measurement();

	auto& renderer = renderer::get_current();

	auto screen_size = camera.visible_world_area;
	vec2i screen_size_i(static_cast<int>(screen_size.x), static_cast<int>(screen_size.y));

	if (clear_current_and_swap_buffers) {
		renderer.clear_current_fbo();
	}

	renderer.set_viewport({ viewport_coordinates.x, viewport_coordinates.y, screen_size_i.x, screen_size_i.y });

	auto character_chased_by_camera = cosmos[viewed_character];

	camera.tick(dt, character_chased_by_camera);
	world_hover_highlighter.update(dt.in_milliseconds());
	
	viewing_step main_cosmos_viewing_step(cosmos, hud, world_hover_highlighter, dt, renderer, camera.get_state_for_drawing_camera(character_chased_by_camera));

	rendering_scripts::standard_rendering(main_cosmos_viewing_step);

	using namespace augs::gui::text;

	const auto coords = character_chased_by_camera.alive() ? character_chased_by_camera.logic_transform().pos : vec2();
	const auto vel = character_chased_by_camera.alive() ? character_chased_by_camera.get<components::physics>().velocity() : vec2();

	auto bbox = quick_print_format(renderer.triangles, typesafe_sprintf(L"Entities: %x\nX: %f2\nY: %f2\nVelX: %x\nVelY: %x\n", cosmos.entities_count(), coords.x, coords.y, vel.x, vel.y)
		+ summary() + cosmos.profiler.sorted_summary(show_profile_details), style(assets::font_id::GUI_FONT, rgba(255, 255, 255, 150)), vec2i(0, 0), 0);

	quick_print(renderer.triangles, multiply_alpha(global_log::format_recent_as_text(assets::font_id::GUI_FONT), 150.f / 255), vec2i(screen_size_i.x - 300, 0), 300);
	quick_print(renderer.triangles, custom_log, vec2i(0, static_cast<int>(bbox.h)), 0);
		
	renderer.call_triangles();
	renderer.clear_triangles();

	triangles.measure(static_cast<double>(renderer.triangles_drawn_total));
	renderer.triangles_drawn_total = 0;

	if (clear_current_and_swap_buffers) {
		window.swap_buffers();
	}

	fps_profiler.end_measurement();
	fps_profiler.new_measurement();
	frame_profiler.end_measurement();
}