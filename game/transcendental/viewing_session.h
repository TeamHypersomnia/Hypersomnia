#pragma once
#include "game/detail/world_camera.h"
#include "augs/misc/variable_delta_timer.h"
#include "augs/misc/measurements.h"

#include "game/global/input_context.h"
#include "game/detail/gui/aabb_highlighter.h"
#include "game/detail/gui/immediate_hud.h"

#include "augs/gui/formatted_text.h"

#include "augs/entity_system/storage_for_systems.h"
#include "game/transcendental/types_specification/all_systems_declaration.h"

#include "game/systems_audiovisual/interpolation_system.h"
#include "game/systems_audiovisual/past_infection_system.h"
#include "game/systems_audiovisual/light_system.h"
#include "game/systems_audiovisual/particles_simulation_system.h"

class game_window;

namespace augs {
	struct machine_entropy;

	namespace network {
		class client;
	}
}

class viewing_session {
public:
	world_camera camera;
	input_context context;
	vec2i viewport_coordinates;
	aabb_highlighter world_hover_highlighter;
	immediate_hud hud;
	storage_for_all_systems_audiovisual systems_audiovisual;

	bool show_profile_details = true;

	void visual_response_from_game_events(const const_logic_step&);

	augs::variable_delta_timer frame_timer;
	augs::measurements fps_profiler = augs::measurements(L"FPS");
	augs::measurements frame_profiler = augs::measurements(L"Frame");
	augs::measurements local_entropy_profiler = augs::measurements(L"Acquiring local entropy");
	augs::measurements unpack_local_steps_profiler = augs::measurements(L"Unpacking local steps");
	augs::measurements unpack_remote_steps_profiler = augs::measurements(L"Unpacking remote steps");
	augs::measurements sending_commands_and_predict_profiler = augs::measurements(L"Sending and predicting commands");
	augs::measurements sending_packets_profiler = augs::measurements(L"Sending packets");
	augs::measurements remote_entropy_profiler = augs::measurements(L"Acquiring remote entropy");
	augs::measurements triangles = augs::measurements(L"Triangles", false);

	std::wstring summary() const;
	
	void reserve_caches_for_entities(const size_t);

	void advance_audiovisual_systems(const cosmos& cosm, const augs::delta dt);
	void resample_state_for_audiovisuals(const cosmos&);

	void control(const augs::machine_entropy&);

	void view(const cosmos& cosmos, 
		const entity_id viewed_character,
		game_window& window, 
		const augs::variable_delta& dt, 
		const augs::gui::text::fstr& custom_log = augs::gui::text::fstr(),
		const bool clear_current_and_swap_buffers = true
		);

	void view(const cosmos& cosmos,
		const entity_id viewed_character,
		game_window& window,
		const augs::variable_delta& dt,
		const augs::network::client& details,
		const bool clear_current_and_swap_buffers = true
	);
};