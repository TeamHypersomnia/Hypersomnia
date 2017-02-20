#pragma once
#include "game/transcendental/cosmic_step.h"

#include "game/detail/visible_entities.h"
#include "game/transcendental/game_drawing_settings.h"

struct aabb_highlighter;
class viewing_session;
struct immediate_hud;
class config_lua_table;

namespace augs {
	class renderer;
}

class viewing_step : public const_cosmic_step {
public:
	viewing_step(
		const config_lua_table& config,
		const cosmos&,
		const viewing_session&,
		const float interpolation_ratio,
		augs::renderer&,
		const camera_cone camera_state,
		const entity_id viewed_character,
		const visible_entities&
	);

	const config_lua_table& config;
	camera_cone camera;
	entity_id viewed_character;
	const visible_entities& visible;
	const float interpolation_ratio = 0.f;

	game_drawing_settings settings;
	const viewing_session& session;
	augs::renderer& renderer;

	double get_interpolated_total_time_passed_in_seconds() const;
	float get_interpolation_ratio() const;

	vec2 get_screen_space(const vec2 pos) const;
};
