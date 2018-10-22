#include "game/modes/arena_mode.h"
#include "view/game_drawing_settings.h"

void arena_mode_view_vars::adjust(game_drawing_settings& settings) const {
	if (fog_of_war_angle > 0.f) {
		auto& fov = settings.fog_of_war;
		fov.enabled = true;
		fov.angle = fog_of_war_angle;
	}

	settings.draw_enemy_hud = show_enemy_hud;
}
