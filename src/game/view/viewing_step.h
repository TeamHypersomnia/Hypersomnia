#pragma once
#include "game/transcendental/cosmic_step.h"

#include "game/detail/visible_entities.h"
#include "game/view/game_drawing_settings.h"

struct aabb_highlighter;
class viewing_session;
class config_lua_table;

namespace augs {
	class renderer;
}

class viewing_step : public const_cosmic_step {
public:
	viewing_step(
		const cosmos&,
		const viewing_session&,
		const double interpolation_ratio,
		augs::renderer&,
		const camera_cone camera_state,
		const entity_id viewed_character,
		const visible_entities&
	);

	const camera_cone camera;
	const entity_id viewed_character;
	const visible_entities& visible;
	const double interpolation_ratio = 0.0;

	const viewing_session& session;
	augs::renderer& renderer;

	double get_interpolated_total_time_passed_in_seconds() const;
	double get_interpolation_ratio() const;
};
