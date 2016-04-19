#pragma once

namespace components {
	struct melee {
		float swing_duration_ms = 200.f;
		float swing_cooldown_ms = 100.f;

		bool primary_move_flag = false;
		bool secondary_move_flag = false;
		bool tertiary_move_flag = false;
	};
}