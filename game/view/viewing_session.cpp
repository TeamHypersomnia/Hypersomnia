#include "viewing_session.h"
#include "application/game_window.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_step.h"
#include "game/view/rendering_scripts/all.h"
#include "augs/misc/machine_entropy.h"
#include "game/components/flags_component.h"
#include "game/messages/item_picked_up_message.h"

#include "augs/network/network_client.h"

viewing_session::viewing_session() {
	systems_audiovisual.get<sound_system>().initialize_sound_sources(32u);
}

void viewing_session::initialize(
	const game_window& window,
	const config_lua_table& cfg
) {
	const vec2i screen_size = vec2i(window.get_screen_size());

	set_screen_size(screen_size);
	systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	set_master_gain(cfg.sound_effects_volume);
	configure_input();
}

void viewing_session::set_screen_size(const vec2i new_size) {
	systems_audiovisual.get<gui_element_system>().screen_size_for_new_characters = new_size;
	camera.configure_size(new_size);
}

void viewing_session::set_interpolation_enabled(const bool flag) {
	systems_audiovisual.get<interpolation_system>().set_interpolation_enabled(flag);
}

void viewing_session::set_master_gain(const float gain) {
	systems_audiovisual.get<sound_system>().master_gain = gain;
}

void viewing_session::configure_input() {
	auto& active_context = context;

	using namespace augs::window::event::keys;
	using namespace augs::window::event;

	active_context.map_key_to_intent(key::W, intent_type::MOVE_FORWARD);
	active_context.map_key_to_intent(key::S, intent_type::MOVE_BACKWARD);
	active_context.map_key_to_intent(key::A, intent_type::MOVE_LEFT);
	active_context.map_key_to_intent(key::D, intent_type::MOVE_RIGHT);

	active_context.map_event_to_intent(message::mousemotion, intent_type::MOVE_CROSSHAIR);
	active_context.map_key_to_intent(key::LMOUSE, intent_type::CROSSHAIR_PRIMARY_ACTION);
	active_context.map_key_to_intent(key::RMOUSE, intent_type::CROSSHAIR_SECONDARY_ACTION);

	active_context.map_key_to_intent(key::E, intent_type::USE_BUTTON);
	active_context.map_key_to_intent(key::LSHIFT, intent_type::SPRINT);

	active_context.map_key_to_intent(key::G, intent_type::THROW);
	active_context.map_key_to_intent(key::H, intent_type::HOLSTER);

	active_context.map_key_to_intent(key::BACKSPACE, intent_type::SWITCH_LOOK);

	active_context.map_key_to_intent(key::LCTRL, intent_type::START_PICKING_UP_ITEMS);

	active_context.map_key_to_intent(key::SPACE, intent_type::SPACE_BUTTON);
	active_context.map_key_to_intent(key::MOUSE4, intent_type::SWITCH_TO_GUI);
	
	active_context.map_key_to_intent(key::CAPSLOCK, intent_type::DEBUG_SWITCH_CHARACTER);

	active_context.map_key_to_intent(key::_1, intent_type::HOTBAR_BUTTON_0);
	active_context.map_key_to_intent(key::_2, intent_type::HOTBAR_BUTTON_1);
	active_context.map_key_to_intent(key::_3, intent_type::HOTBAR_BUTTON_2);
	active_context.map_key_to_intent(key::_4, intent_type::HOTBAR_BUTTON_3);
	active_context.map_key_to_intent(key::_5, intent_type::HOTBAR_BUTTON_4);
	active_context.map_key_to_intent(key::_6, intent_type::HOTBAR_BUTTON_5);
	active_context.map_key_to_intent(key::_7, intent_type::HOTBAR_BUTTON_6);
	active_context.map_key_to_intent(key::_8, intent_type::HOTBAR_BUTTON_7);
	active_context.map_key_to_intent(key::_9, intent_type::HOTBAR_BUTTON_8);
	active_context.map_key_to_intent(key::_0, intent_type::HOTBAR_BUTTON_9);

	active_context.map_key_to_intent(key::Z, intent_type::SPECIAL_ACTION_BUTTON_1);
	active_context.map_key_to_intent(key::X, intent_type::SPECIAL_ACTION_BUTTON_2);
	active_context.map_key_to_intent(key::C, intent_type::SPECIAL_ACTION_BUTTON_3);
	active_context.map_key_to_intent(key::V, intent_type::SPECIAL_ACTION_BUTTON_4);
	active_context.map_key_to_intent(key::B, intent_type::SPECIAL_ACTION_BUTTON_5);
	active_context.map_key_to_intent(key::N, intent_type::SPECIAL_ACTION_BUTTON_6);
	active_context.map_key_to_intent(key::M, intent_type::SPECIAL_ACTION_BUTTON_7);
	active_context.map_key_to_intent(key::F2, intent_type::SPECIAL_ACTION_BUTTON_8);
	active_context.map_key_to_intent(key::F3, intent_type::SPECIAL_ACTION_BUTTON_9);
	active_context.map_key_to_intent(key::F4, intent_type::SPECIAL_ACTION_BUTTON_10);
	active_context.map_key_to_intent(key::F5, intent_type::SPECIAL_ACTION_BUTTON_11);
	active_context.map_key_to_intent(key::F6, intent_type::SPECIAL_ACTION_BUTTON_12);

	active_context.map_key_to_intent(key::Q, intent_type::PREVIOUS_HOTBAR_SELECTION_SETUP);

	active_context.map_key_to_intent(key::F, intent_type::SWITCH_WEAPON_LASER);
	active_context.map_key_to_intent(key::DASH, intent_type::OPEN_DEVELOPER_CONSOLE);
}

void viewing_session::reserve_caches_for_entities(const size_t n) {
	systems_audiovisual.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void viewing_session::switch_between_gui_and_back(const augs::machine_entropy::local_type& local) {
	auto& gui = systems_audiovisual.get<gui_element_system>();

	for (const auto& intent : context.to_key_and_mouse_intents(local)) {
		if (intent.is_pressed && intent.intent == intent_type::SWITCH_TO_GUI) {
			gui.gui_look_enabled = !gui.gui_look_enabled;
		}
	}
}

void viewing_session::control_gui_and_remove_fetched_events(
	const const_entity_handle root,
	augs::machine_entropy::local_type& entropies
) {
	auto& gui = systems_audiovisual.get<gui_element_system>();

	gui.control_gui(
		root, 
		entropies
	);

	gui.handle_hotbar_and_action_button_presses(
		root,
		context.to_key_and_mouse_intents(entropies)
	);
}

void viewing_session::control_and_remove_fetched_intents(key_and_mouse_intent_vector& intents) {
	erase_remove(intents, [&](const key_and_mouse_intent& intent) {
		bool fetch = false;

		if (intent.intent == intent_type::SWITCH_LOOK) {
			augs::renderer::get_current().persistent_lines.lines.clear();
		}

		if (intent.intent == intent_type::OPEN_DEVELOPER_CONSOLE) {
			fetch = true;

			if (intent.is_pressed) {
				show_profile_details = !show_profile_details;
			}
		}
		else if (intent.intent == intent_type::SWITCH_WEAPON_LASER) {
			fetch = true;

			if (intent.is_pressed) {
				drawing_settings.draw_weapon_laser = !drawing_settings.draw_weapon_laser;
			}
		}

		return fetch;
	});
}

void viewing_session::spread_past_infection(const const_logic_step step) {
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

void viewing_session::advance_audiovisual_systems(
	const cosmos& cosm, 
	const entity_id viewed_character,
	const visible_entities& all_visible,
	const augs::delta dt
) {
	auto& thunders = systems_audiovisual.get<thunder_system>();
	auto& exploding_rings = systems_audiovisual.get<exploding_ring_system>();
	auto& flying_numbers = systems_audiovisual.get<vertically_flying_number_system>();
	auto& highlights = systems_audiovisual.get<pure_color_highlight_system>();
	auto& interp = systems_audiovisual.get<interpolation_system>();
	auto& particles = systems_audiovisual.get<particles_simulation_system>();

	thunders.advance(cosm, dt, particles);
	exploding_rings.advance(cosm, dt, particles);
	flying_numbers.advance(dt);
	highlights.advance(dt);

	cosm.profiler.start(meter_type::INTERPOLATION);
	interp.integrate_interpolated_transforms(cosm, dt, cosm.get_fixed_delta());
	cosm.profiler.stop(meter_type::INTERPOLATION);

	particles.advance_visible_streams_and_all_particles(
		camera.smoothed_camera, 
		cosm, 
		dt, 
		interp
	);
	
	systems_audiovisual.get<light_system>().advance_attenuation_variations(cosm, dt);

	camera.tick(
		interp, 
		dt, 
		cosm[viewed_character]
	);
	
	systems_audiovisual.get<wandering_pixels_system>().advance_for_visible(
		all_visible, 
		cosm,
		dt
	);
	
	world_hover_highlighter.cycle_duration_ms = 400;
	world_hover_highlighter.update(dt.in_milliseconds());

	auto& gui = systems_audiovisual.get<gui_element_system>();

	gui.advance_elements(
		cosm[viewed_character],
		dt
	);

	gui.rebuild_layouts(
		cosm[viewed_character]
	);

	auto listener_cone = camera.smoothed_camera;
	listener_cone.transform = cosm[viewed_character].get_viewing_transform(interp);

	systems_audiovisual.get<sound_system>().play_nearby_sound_existences(
		listener_cone,
		viewed_character,
		cosm,
		interp,
		dt
	);
}

void viewing_session::draw_text_at_left_top(
	augs::renderer& renderer,
	const augs::gui::text::formatted_string& str
) const {
	quick_print(renderer.triangles, str, vec2i(0, 0), 0);
}

void viewing_session::view(
	const config_lua_table& config,
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const visible_entities& all_visible,
	const double interpolation_ratio,
	const augs::network::client& details
) const {
	using namespace augs::gui::text;
	
	const auto custom_log = multiply_alpha(format_as_bbcode(typesafe_sprintf("[color=cyan]Transmission details:[/color]\n%x", details.format_transmission_details()), style(assets::font_id::GUI_FONT, white)), 150.f / 255);;

	view(config, renderer, cosmos, viewed_character, all_visible, interpolation_ratio, custom_log);
}

void viewing_session::view(
	const config_lua_table& config,
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const visible_entities& all_visible,
	const double interpolation_ratio,
	const augs::gui::text::formatted_string& custom_log
) const {
	frame_profiler.new_measurement();

	const auto screen_size = camera.smoothed_camera.visible_world_area;
	const vec2i screen_size_i(static_cast<int>(screen_size.x), static_cast<int>(screen_size.y));

	renderer.set_viewport({ viewport_coordinates.x, viewport_coordinates.y, screen_size_i.x, screen_size_i.y });

	const auto character_chased_by_camera = cosmos[viewed_character];

	auto main_cosmos_viewing_step = viewing_step(
		config,
		cosmos, 
		*this, 
		interpolation_ratio,
		renderer, 
		camera.smoothed_camera, 
		character_chased_by_camera,
		all_visible
	);

	main_cosmos_viewing_step.settings = drawing_settings;

#if NDEBUG || _DEBUG
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#else
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#endif

	using namespace augs::gui::text;

	if (show_profile_details) {
		quick_print(
			renderer.triangles, 
			multiply_alpha(global_log::format_recent_as_text(assets::font_id::GUI_FONT), 150.f / 255), 
			vec2i(screen_size_i.x - 300, 0), 
			300
		);

		const auto coords = character_chased_by_camera.alive() ? character_chased_by_camera.get_logic_transform().pos : vec2();
		const auto rot = character_chased_by_camera.alive() ? character_chased_by_camera.get_logic_transform().rotation : 0.f;
		const auto vel = character_chased_by_camera.alive() ? character_chased_by_camera.get<components::rigid_body>().velocity() : vec2();

		const auto gui_style = style(
			assets::font_id::GUI_FONT,
			rgba(255, 255, 255, 150)
		);

		const auto lt_text_formatted = format(
			typesafe_sprintf(
				L"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\nVelX: %x\nVelY: %x\n",
				cosmos.entities_count(),
				coords.x,
				coords.y,
				rot,
				vel.x,
				vel.y
			) + summary() + cosmos.profiler.sorted_summary(show_profile_details) + L"\n",

			gui_style
		);

		draw_text_at_left_top(renderer, lt_text_formatted + custom_log);
	}
		
	renderer.call_triangles();
	renderer.clear_triangles();

	triangles.measure(static_cast<double>(renderer.triangles_drawn_total));
	renderer.triangles_drawn_total = 0;

	fps_profiler.end_measurement();
	fps_profiler.new_measurement();
	frame_profiler.end_measurement();
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

void viewing_session::draw_color_overlay(augs::renderer& renderer, const rgba col) const {
	components::sprite overlay;
	overlay.set(assets::game_image_id::BLANK, camera.smoothed_camera.visible_world_area, col);

	components::sprite::drawing_input in(renderer.get_triangle_buffer());
	in.camera = camera.smoothed_camera;
	in.renderable_transform = camera.smoothed_camera.transform;

	overlay.draw(in);

	renderer.call_triangles();
	renderer.clear_triangles();
}

void viewing_session::get_visible_entities(
	visible_entities& into,
	const cosmos& cosm
) {
	into.from_camera(camera.smoothed_camera, cosm);
}