#pragma once
#include <optional>

#include "augs/math/camera_cone.h"
#include "game/cosmos/entity_handle_declaration.h"

struct intercosm;
struct debugger_view;
class debugger_player;
struct debugger_camera_settings;

namespace debugger_detail {
	std::optional<camera_eye> calculate_camera(
		const debugger_player& player,
		const debugger_view& view,
		const_entity_handle matching_go_to_entity,
		const_entity_handle viewed_character
	);

	bool handle_camera_input(
		const debugger_camera_settings& settings,
		camera_cone current_cone,
		const augs::event::state& common_input_state,
		const augs::event::change e,
		vec2 world_cursor_pos,
		std::optional<camera_eye>& panned_camera
	);
}
