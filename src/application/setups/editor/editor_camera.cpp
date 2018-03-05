#include "application/intercosm.h"

#include "application/setups/editor/editor_camera.h"
#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/editor_player.h"

namespace editor_detail {
	std::optional<camera_cone> calculate_camera(
		const editor_player& player,
		const editor_view& view,
		const_entity_handle matching_go_to_entity,
		const intercosm& icm
	) {
		std::optional<components::transform> viewed_transform;

		const auto panning = view.panned_camera;
		const auto viewed_character = icm.get_viewed_character();

		if (viewed_character) {
			viewed_transform = viewed_character.find_logic_transform();
		}

		if (player.is_editing_mode()) {
			if (const auto match = matching_go_to_entity) {
				camera_cone centered_on_match;

				if (panning) {
					/* Propagate zoom taken from custom panning */
					centered_on_match = *panning;
				}

				if (const auto transform = match.find_logic_transform()) {
					centered_on_match.transform.pos = transform->pos;
				}
				else {
					LOG("WARNING: transform of %x could not be found.", match);
				}

				return centered_on_match;
			}
			else if (panning) {
				return panning;
			}

			if (viewed_transform) {
				camera_cone centered_on_viewed;
				centered_on_viewed.transform.pos = viewed_transform->pos;

				return centered_on_viewed;
			}

			return camera_cone();
		}

		if (panning) {
			return panning;
		}

		if (!viewed_character) {
			return camera_cone();
		}

		return std::nullopt;
	}
}
