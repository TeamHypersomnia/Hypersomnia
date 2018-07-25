#pragma once
#include <optional>

#include "augs/math/camera_cone.h"
#include "game/cosmos/entity_handle_declaration.h"

struct intercosm;
struct editor_view;
struct editor_player;
struct editor_camera_settings;

namespace editor_detail {
	std::optional<camera_eye> calculate_camera(
		const editor_player& player,
		const editor_view& view,
		const_entity_handle matching_go_to_entity,
		const intercosm& icm
	);

	bool handle_camera_input(
		const editor_camera_settings& settings,
		camera_cone current_cone,
		const augs::event::state& common_input_state,
		const augs::event::change e,
		vec2 world_cursor_pos,
		std::optional<camera_eye>& panned_camera
	);
}
