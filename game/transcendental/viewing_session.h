#pragma once
#include "game/detail/world_camera.h"
#include "augs/misc/variable_delta_timer.h"
#include "augs/misc/measurements.h"

#include "game/global/input_context.h"
#include "game/detail/gui/aabb_highlighter.h"
#include "game/detail/gui/immediate_hud.h"

#include "augs/gui/formatted_text.h"

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

	bool show_profile_details = false;

	void visual_response_to_game_events(const logic_step&);

	augs::variable_delta_timer frame_timer;
	augs::measurements fps_profiler = augs::measurements(L"Frame");
	augs::measurements triangles = augs::measurements(L"Triangles", false);

	std::wstring summary() const;

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