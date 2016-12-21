#include "viewing_session.h"
#include "application/game_window.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/scene_managers/rendering_scripts/all.h"
#include "augs/misc/machine_entropy.h"
#include "game/components/flags_component.h"

#include "augs/network/network_client.h"

viewing_session::viewing_session() {
	systems_audiovisual.get<sound_system>().initialize_sound_sources(32u);
}

std::wstring viewing_session::summary() const {
	return 
		fps_profiler.summary()
		+ frame_profiler.summary()
		+ triangles.summary()
		+ local_entropy_profiler.summary()
		+ remote_entropy_profiler.summary()
		+ unpack_local_steps_profiler.summary()
		+ sending_commands_and_predict_profiler.summary()
		+ unpack_remote_steps_profiler.summary()
		+ sending_packets_profiler.summary()
		;
}

void viewing_session::acquire_game_events_for_hud(const const_logic_step& step) {
	hud.acquire_game_events(step);
}

void viewing_session::set_interpolation_enabled(const bool flag) {
	systems_audiovisual.get<interpolation_system>().enabled = flag;
}

void viewing_session::set_master_gain(const float gain) {
	systems_audiovisual.get<sound_system>().master_gain = gain;
}

void viewing_session::spread_past_infection(const const_logic_step& step) {
	const auto& cosm = step.cosm;

	const auto& events = step.transient.messages.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		const const_entity_handle subject_owner_body = cosm[it.subject].get_owner_body();
		const const_entity_handle collider_owner_body = cosm[it.collider].get_owner_body();

		auto& past_system = systems_audiovisual.get<past_infection_system>();

		if (past_system.is_infected(subject_owner_body) && !collider_owner_body.get_flag(entity_flag::IS_IMMUNE_TO_PAST)) {
			past_system.infect(collider_owner_body);
		}
	}
}

void viewing_session::reserve_caches_for_entities(const size_t n) {
	systems_audiovisual.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void viewing_session::advance_audiovisual_systems(
	const cosmos& cosm, 
	const entity_id viewed_character,
	const augs::variable_delta dt
) {
	auto& interp = systems_audiovisual.get<interpolation_system>();

	cosm.profiler.start(meter_type::INTERPOLATION);
	interp.integrate_interpolated_transforms(cosm, dt, dt.get_fixed());
	cosm.profiler.stop(meter_type::INTERPOLATION);

	systems_audiovisual.get<particles_simulation_system>().advance_visible_streams_and_all_particles(
		camera.smoothed_camera, 
		cosm, 
		dt, 
		interp);
	
	auto listener_cone = camera.smoothed_camera;
	listener_cone.transform = cosm[viewed_character].viewing_transform(interp);

	systems_audiovisual.get<sound_system>().play_nearby_sound_existences(
		listener_cone,
		viewed_character,
		cosm,
		cosm.get_total_time_passed_in_seconds() + dt.seconds_after_last_step(),
		interp
	);
}

void viewing_session::resample_state_for_audiovisuals(const cosmos& cosm) {
	systems_audiovisual.for_each([&cosm](auto& sys) {
		sys.resample_state_for_audiovisuals(cosm);
	});
}

void viewing_session::control(const augs::machine_entropy& entropy) {
	for (const auto& raw_input : entropy.local) {
		if (raw_input.was_key_pressed(augs::window::event::keys::key::DASH)) {
			show_profile_details = !show_profile_details;
		}
	}
}

void viewing_session::view(
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const augs::variable_delta& dt,
	const augs::network::client& details,
	const game_drawing_settings settings
) {
	using namespace augs::gui::text;
	
	const auto custom_log = multiply_alpha(simple_bbcode(typesafe_sprintf("[color=cyan]Transmission details:[/color]\n%x", details.format_transmission_details()), style(assets::font_id::GUI_FONT, white)), 150.f / 255);;

	view(renderer, cosmos, viewed_character, dt, custom_log);
}

void viewing_session::view(
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const augs::variable_delta& dt,
	const augs::gui::text::fstr& custom_log,
	const game_drawing_settings settings
	) {
	frame_profiler.new_measurement();

	const auto screen_size = camera.camera.visible_world_area;
	const vec2i screen_size_i(static_cast<int>(screen_size.x), static_cast<int>(screen_size.y));

	renderer.set_viewport({ viewport_coordinates.x, viewport_coordinates.y, screen_size_i.x, screen_size_i.y });

	const auto character_chased_by_camera = cosmos[viewed_character];

	camera.tick(systems_audiovisual.get<interpolation_system>(), dt, character_chased_by_camera);
	world_hover_highlighter.cycle_duration_ms = 700;
	world_hover_highlighter.update(dt.in_milliseconds());
	
	viewing_step main_cosmos_viewing_step(cosmos, *this, dt, renderer, camera.smoothed_camera, character_chased_by_camera);
	main_cosmos_viewing_step.settings = settings;

#if NDEBUG || _DEBUG
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#else
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#endif

	using namespace augs::gui::text;

	if (show_profile_details) {
		const auto coords = character_chased_by_camera.alive() ? character_chased_by_camera.logic_transform().pos : vec2();
		const auto rot = character_chased_by_camera.alive() ? character_chased_by_camera.logic_transform().rotation : 0.f;
		const auto vel = character_chased_by_camera.alive() ? character_chased_by_camera.get<components::physics>().velocity() : vec2();

		const auto bbox = quick_print_format(renderer.triangles, typesafe_sprintf(L"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\nVelX: %x\nVelY: %x\n", cosmos.entities_count(), coords.x, coords.y, rot, vel.x, vel.y)
			+ summary() + cosmos.profiler.sorted_summary(show_profile_details), style(assets::font_id::GUI_FONT, rgba(255, 255, 255, 150)), vec2i(0, 0), 0);

		quick_print(renderer.triangles, multiply_alpha(global_log::format_recent_as_text(assets::font_id::GUI_FONT), 150.f / 255), vec2i(screen_size_i.x - 300, 0), 300);
		quick_print(renderer.triangles, custom_log, vec2i(0, static_cast<int>(bbox.h)), 0);
	}
		
	renderer.call_triangles();
	renderer.clear_triangles();

	triangles.measure(static_cast<double>(renderer.triangles_drawn_total));
	renderer.triangles_drawn_total = 0;

	fps_profiler.end_measurement();
	fps_profiler.new_measurement();
	frame_profiler.end_measurement();
}

void viewing_session::draw_color_overlay(augs::renderer& renderer, const rgba col) const {
	components::sprite overlay;
	overlay.set(assets::texture_id::BLANK, col);
	overlay.size = camera.smoothed_camera.visible_world_area;

	components::sprite::drawing_input in(renderer.get_triangle_buffer());
	in.camera = camera.smoothed_camera;
	in.renderable_transform = camera.smoothed_camera.transform;

	overlay.draw(in);

	renderer.call_triangles();
	renderer.clear_triangles();
}