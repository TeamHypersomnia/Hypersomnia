#include "viewing_session.h"
#include "application/game_window.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_step.h"
#include "game/view/rendering_scripts/all.h"
#include "augs/misc/machine_entropy.h"
#include "game/components/flags_component.h"
#include "game/messages/item_picked_up_message.h"

#include "augs/network/network_client.h"
#include "hypersomnia_version.h"
#include "game/build_settings.h"

#include "augs/misc/lua_readwrite.h"
#include "generated/introspectors.h"
#include "augs/filesystem/file.h"
#include "augs/misc/imgui_utils.h"

viewing_session::viewing_session(
	const vec2i screen_size,
	const config_lua_table& config
) : 
	config(config), 
	last_saved_config(config) 
{
	systems_audiovisual.get<sound_system>().initialize_sound_sources(32u);
	
	set_screen_size(screen_size);
	systems_audiovisual.get<interpolation_system>().interpolation_speed = static_cast<float>(config.interpolation_speed);
	set_master_gain(static_cast<float>(config.sound_effects_volume));

	auto& io = ImGui::GetIO();
	
	using namespace augs::window::event::keys;

	auto map_key = [&io](auto im, auto aug) {
		io.KeyMap[im] = int(aug);
	};

	map_key(ImGuiKey_Tab, key::TAB);
	map_key(ImGuiKey_LeftArrow, key::LEFT);
	map_key(ImGuiKey_RightArrow, key::RIGHT);
	map_key(ImGuiKey_UpArrow, key::UP);
	map_key(ImGuiKey_DownArrow, key::DOWN);
	map_key(ImGuiKey_PageUp, key::PAGEUP);
	map_key(ImGuiKey_PageDown, key::PAGEDOWN);
	map_key(ImGuiKey_Home, key::HOME);
	map_key(ImGuiKey_End, key::END);
	map_key(ImGuiKey_Delete, key::DEL);
	map_key(ImGuiKey_Backspace, key::BACKSPACE);
	map_key(ImGuiKey_Enter, key::ENTER);
	map_key(ImGuiKey_Escape, key::ESC);
	map_key(ImGuiKey_A, key::A);
	map_key(ImGuiKey_C, key::C);
	map_key(ImGuiKey_V, key::V);
	map_key(ImGuiKey_X, key::X);
	map_key(ImGuiKey_Y, key::Y);
	map_key(ImGuiKey_Z, key::Z);

	io.IniFilename = "generated/imgui.ini";
	io.LogFilename = "generated/imgui_log.txt";
	io.MouseDoubleClickMaxDist = 100.f;

	ImGui::GetStyle() = config.gui_style;
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

void viewing_session::reserve_caches_for_entities(const size_t n) {
	systems_audiovisual.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void viewing_session::switch_between_gui_and_back(const augs::machine_entropy::local_type& local) {
	auto& gui = systems_audiovisual.get<gui_element_system>();

	for (const auto& intent : config.controls.translate(local).intents) {
		if (intent.is_pressed && intent.intent == intent_type::SWITCH_TO_GUI) {
			gui.gui_look_enabled = !gui.gui_look_enabled;
		}
	}
}

void viewing_session::control_gui_and_remove_fetched_events(
	const const_entity_handle root,
	augs::machine_entropy::local_type& window_inputs
) {
	if (root.alive()) {
		auto& gui = systems_audiovisual.get<gui_element_system>();

		gui.control_gui(
			root, 
			window_inputs
		);

		gui.handle_hotbar_and_action_button_presses(
			root,
			config.controls.translate(window_inputs).intents
		);
	}
	
}

void viewing_session::perform_imgui_pass(
	augs::window::glwindow& window,
	const augs::machine_entropy::local_type& window_inputs,
	const augs::delta dt
) {
	const auto size = camera.camera.visible_world_area;

	auto& io = ImGui::GetIO();

	using namespace augs::window::event;
	using namespace augs::window::event::keys;
	
	io.MouseDrawCursor = false;

	for (const auto& in : window_inputs) {
		if (in.msg == message::mousemotion) {
			io.MousePos = (vec2(in.mouse.rel) + io.MousePos).clamp_from_zero_to(size);
		}
		else if (in.msg == message::ldown || in.msg == message::ldoubleclick || in.msg == message::ltripleclick) {
			io.MouseDown[0] = true;
		}
		else if (in.msg == message::lup) {
			io.MouseDown[0] = false;
		}
		else if (in.msg == message::rdown || in.msg == message::rdoubleclick) {
			io.MouseDown[1] = true;
		}
		else if (in.msg == message::rup) {
			io.MouseDown[1] = false;
		}
		else if (in.msg == message::wheel) {
			io.MouseWheel = in.scroll.amount;
		}
		else if (in.msg == message::keydown) {
			io.KeysDown[int(in.key.key)] = true;
		}
		else if (in.msg == message::keyup) {
			io.KeysDown[int(in.key.key)] = false;
		}
		else if (in.msg == message::character) {
			io.AddInputCharacter(in.character.utf16);
		}
		else if (in.msg == message::activate) {
			for (auto& k : io.KeysDown) {
				k = false;
			}

			for (auto& m : io.MouseDown) {
				m = false;
			}

			io.MouseWheel = false;
		}

		io.KeyCtrl = io.KeysDown[int(keys::key::LCTRL)] || io.KeysDown[int(keys::key::RCTRL)];
		io.KeyShift = io.KeysDown[int(keys::key::LSHIFT)] || io.KeysDown[int(keys::key::RSHIFT)];
		io.KeyAlt = io.KeysDown[int(keys::key::LALT)] || io.KeysDown[int(keys::key::RALT)];
	}

	io.DeltaTime = dt.in_seconds();
	io.DisplaySize = size;

	ImGui::NewFrame();
	
	perform_settings_gui(window);

	ImGui::Render();
}

void viewing_session::control_open_developer_console(game_intent_vector& intents) {
	erase_if(intents, [&](const game_intent& intent) {
		bool fetch = false;

		if (intent.intent == intent_type::OPEN_DEVELOPER_CONSOLE) {
			fetch = true;

			if (intent.is_pressed) {
				config.drawing_settings.show_profile_details = !config.drawing_settings.show_profile_details;
			}
		}

		return fetch;
	});
}

void viewing_session::control_and_remove_fetched_intents(game_intent_vector& intents) {
	control_open_developer_console(intents);

	erase_if(intents, [&](const game_intent& intent) {
		bool fetch = false;

		if (intent.intent == intent_type::CLEAR_DEBUG_LINES) {
			augs::renderer::get_current().persistent_lines.lines.clear();
		}

		if (intent.intent == intent_type::OPEN_DEVELOPER_CONSOLE) {
			fetch = true;

			if (intent.is_pressed) {
				config.drawing_settings.show_profile_details = !config.drawing_settings.show_profile_details;
			}
		}
		else if (intent.intent == intent_type::SWITCH_WEAPON_LASER) {
			fetch = true;

			if (intent.is_pressed) {
				config.drawing_settings.draw_weapon_laser = !config.drawing_settings.draw_weapon_laser;
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
	const entity_id viewed_character_id,
	const visible_entities& all_visible,
	const augs::delta dt
) {
	reserve_caches_for_entities(cosm.get_aggregate_pool().capacity());

	auto& thunders = systems_audiovisual.get<thunder_system>();
	auto& exploding_rings = systems_audiovisual.get<exploding_ring_system>();
	auto& flying_numbers = systems_audiovisual.get<flying_number_indicator_system>();
	auto& highlights = systems_audiovisual.get<pure_color_highlight_system>();
	auto& interp = systems_audiovisual.get<interpolation_system>();
	auto& particles = systems_audiovisual.get<particles_simulation_system>();

	const auto viewed_character = cosm[viewed_character_id];

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
		config.camera_settings,
		viewed_character
	);
	
	systems_audiovisual.get<wandering_pixels_system>().advance_for_visible(
		all_visible, 
		cosm,
		dt
	);
	
	world_hover_highlighter.cycle_duration_ms = 400;
	world_hover_highlighter.update(dt.in_milliseconds());

	if (viewed_character.alive()) {
		auto& gui = systems_audiovisual.get<gui_element_system>();

		gui.advance_elements(
			viewed_character,
			dt
		);

		gui.rebuild_layouts(
			viewed_character
		);

		auto listener_cone = camera.smoothed_camera;
		listener_cone.transform = viewed_character.get_viewing_transform(interp);

		systems_audiovisual.get<sound_system>().play_nearby_sound_existences(
			listener_cone,
			viewed_character,
			cosm,
			interp,
			dt
		);
	}
}

void viewing_session::draw_text_at_left_top(
	augs::renderer& renderer,
	const augs::gui::text::formatted_string& str
) const {
	quick_print(renderer.triangles, str, vec2i(0, 0), 0);
}

void viewing_session::view(
	augs::renderer& renderer,
	const cosmos& cosmos,
	const entity_id viewed_character,
	const visible_entities& all_visible,
	const double interpolation_ratio,
	const augs::network::client& details
) const {
	using namespace augs::gui::text;
	
	const auto custom_log = multiply_alpha(format_as_bbcode(typesafe_sprintf("[color=cyan]Transmission details:[/color]\n%x", details.format_transmission_details()), style(assets::font_id::GUI_FONT, white)), 150.f / 255);;

	view(renderer, cosmos, viewed_character, all_visible, interpolation_ratio, custom_log);
}

void viewing_session::view(
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
		cosmos, 
		*this, 
		interpolation_ratio,
		renderer, 
		camera.smoothed_camera, 
		character_chased_by_camera,
		all_visible
	);

#if NDEBUG || _DEBUG
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#else
	rendering_scripts::illuminated_rendering(main_cosmos_viewing_step);
#endif

	using namespace augs::gui::text;

	if (config.drawing_settings.show_profile_details) {
		quick_print(
			renderer.triangles, 
			multiply_alpha(augs::gui::text::format_recent_global_log(assets::font_id::GUI_FONT), 150.f / 255), 
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

		const auto revision_info =
			augs::gui::text::format_as_bbcode(
				typesafe_sprintf(
					"Revision no.: %x %x\nDate: %x\nMessage:\n%x\n",
					HYPERSOMNIA_COMMIT_NUMBER,
					HYPERSOMNIA_WORKING_TREE_CHANGES.empty() ? "(clean)" : "(dirty)",
					HYPERSOMNIA_COMMIT_DATE,
					HYPERSOMNIA_COMMIT_MESSAGE.size() < 30 ? HYPERSOMNIA_COMMIT_MESSAGE : HYPERSOMNIA_COMMIT_MESSAGE.substr(0, 30) + "(...)"
				),
				gui_style
			)
		;

		const auto lt_text_formatted = revision_info + format(
			typesafe_sprintf(
				L"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\nVelX: %x\nVelY: %x\n",
				cosmos.get_entities_count(),
				coords.x,
				coords.y,
				rot,
				vel.x,
				vel.y
			) + summary() + cosmos.profiler.sorted_summary(config.drawing_settings.show_profile_details) + L"\n",

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