#pragma once
#include <imgui/imgui.h>

#include "game/view/world_camera.h"
#include "augs/misc/measurements.h"

#include "augs/misc/basic_input_context.h"
#include "augs/misc/fixed_delta_timer.h"

#include "game/detail/gui/aabb_highlighter.h"

#include "augs/gui/formatted_text.h"

#include "augs/entity_system/storage_for_systems.h"
#include "game/transcendental/types_specification/all_systems_declaration.h"
#include "game/transcendental/logic_step.h"

#include "game/systems_audiovisual/interpolation_system.h"
#include "game/systems_audiovisual/past_infection_system.h"
#include "game/systems_audiovisual/light_system.h"
#include "game/systems_audiovisual/particles_simulation_system.h"
#include "game/systems_audiovisual/wandering_pixels_system.h"
#include "game/systems_audiovisual/sound_system.h"
#include "game/systems_audiovisual/gui_element_system.h"
#include "game/systems_audiovisual/flying_number_indicator_system.h"
#include "game/systems_audiovisual/pure_color_highlight_system.h"
#include "game/systems_audiovisual/exploding_ring_system.h"
#include "game/systems_audiovisual/thunder_system.h"

#include "game/detail/gui/character_gui.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"

#include "game/detail/particle_types.h"

class game_window;

namespace augs {
	struct machine_entropy;

	namespace network {
		class client;
	}

	class renderer;
}

class viewing_session {
public:
	config_lua_table config;
	ImGuiStyle gui_style;

	world_camera camera;
	vec2i viewport_coordinates;
	aabb_highlighter world_hover_highlighter;
	storage_for_all_systems_audiovisual systems_audiovisual;

	bool gui_look_enabled = false;

	augs::timer frame_timer;
	augs::timer imgui_timer;

	mutable augs::measurements fps_profiler = augs::measurements(L"FPS");
	mutable augs::measurements frame_profiler = augs::measurements(L"Frame");
	mutable augs::measurements local_entropy_profiler = augs::measurements(L"Acquiring local entropy");
	mutable augs::measurements unpack_local_steps_profiler = augs::measurements(L"Unpacking local steps");
	mutable augs::measurements unpack_remote_steps_profiler = augs::measurements(L"Unpacking remote steps");
	mutable augs::measurements sending_commands_and_predict_profiler = augs::measurements(L"Sending and predicting commands");
	mutable augs::measurements sending_packets_profiler = augs::measurements(L"Sending packets");
	mutable augs::measurements remote_entropy_profiler = augs::measurements(L"Acquiring remote entropy");
	mutable augs::measurements triangles = augs::measurements(L"Triangles", false);

	viewing_session(
		const vec2i screen_size,
		const config_lua_table&
	);

	void set_screen_size(const vec2i);

	void set_interpolation_enabled(const bool);
	void set_master_gain(const float);

	void reserve_caches_for_entities(const size_t);
	void switch_between_gui_and_back(const augs::machine_entropy::local_type&);
	
	void control_gui_and_remove_fetched_events(
		const const_entity_handle root,
		augs::machine_entropy::local_type&
	);
	
	void perform_imgui_pass(
		const augs::machine_entropy::local_type&,
		const augs::delta dt
	);

	void control_open_developer_console(game_intent_vector&);
	void control_and_remove_fetched_intents(game_intent_vector&);
	void standard_audiovisual_post_solve(const const_logic_step);
	void spread_past_infection(const const_logic_step);

	decltype(auto) get_standard_post_solve() {
		return [this](const const_logic_step step) {
			standard_audiovisual_post_solve(step);
		};
	}

	void advance_audiovisual_systems(
		const cosmos& cosm, 
		const entity_id viewed_character,
		const visible_entities&,
		const augs::delta dt
	);
	
	void view(
		augs::renderer& renderer,
		const cosmos& cosmos,
		const entity_id viewed_character,
		const visible_entities&,
		const double interpolation_ratio,
		const augs::gui::text::formatted_string& custom_log = augs::gui::text::formatted_string()
	) const;

	void view(
		augs::renderer& renderer,
		const cosmos& cosmos,
		const entity_id viewed_character,
		const visible_entities&,
		const double interpolation_ratio,
		const augs::network::client& details
	) const;

	void draw_text_at_left_top(
		augs::renderer& renderer,
		const augs::gui::text::formatted_string&
	) const;

	void get_visible_entities(
		visible_entities& into,
		const cosmos&
	);
	
	std::wstring summary() const;

	void draw_color_overlay(
		augs::renderer& renderer, 
		const rgba
	) const;
};