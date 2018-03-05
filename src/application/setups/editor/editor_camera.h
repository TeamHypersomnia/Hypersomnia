#pragma once
#include <optional>

#include "augs/math/camera_cone.h"
#include "game/transcendental/entity_handle_declaration.h"

struct intercosm;
struct editor_view;
struct editor_player;

namespace editor_detail {
	std::optional<camera_cone> calculate_camera(
		const editor_player& player,
		const editor_view& view,
		const_entity_handle matching_go_to_entity,
		const intercosm& icm
	);
}
