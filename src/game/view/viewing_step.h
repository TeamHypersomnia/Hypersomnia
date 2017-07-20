#pragma once
#include "game/transcendental/cosmic_step.h"

#include "game/detail/visible_entities.h"
#include "game/detail/gui/hotbar_settings.h"
#include "game/view/game_drawing_settings.h"
#include "game/enums/input_context_enums.h"
#include "augs/misc/basic_input_context.h"

struct aabb_highlighter;
class viewing_session;
class config_lua_table;

struct audiovisual_state;

namespace augs {
	class renderer;
}

class viewing_step : public const_cosmic_step {
public:
	viewing_step(
		const cosmos&,
		const audiovisual_state&,
		const game_drawing_settings,
		const hotbar_settings,
		const input_context,
		const double interpolation_ratio,
		augs::renderer&,
		const camera_cone camera_state,
		const entity_id viewed_character,
		const visible_entities&,
		const bool is_ingame_menu_active
	);

	const camera_cone camera;
	const entity_id viewed_character;
	const visible_entities& visible;
	const double interpolation_ratio = 0.0;
	const game_drawing_settings drawing;
	const hotbar_settings hotbar;
	const input_context input_information;
	const bool is_ingame_menu_active;

	const audiovisual_state& audiovisuals;
	augs::renderer& renderer;

	double get_interpolated_total_time_passed_in_seconds() const;
	double get_interpolation_ratio() const;
};
