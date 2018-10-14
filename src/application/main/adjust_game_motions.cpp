#include "application/main/adjust_game_motions.h"

void adjust_game_motions(
	vec2 simulated_offset,
	const vec2& sensitivity,
	vec2i screen_size,
	game_motions& motions
) {
	const auto half_bound = vec2(screen_size);

	for (auto& m : motions) {
		if (m.motion == game_motion_type::MOVE_CROSSHAIR) {
			auto adjust_coord = [&](auto& off, const auto& sens, const auto& current, const auto& bound) {
				if (off > 0) {
					while (off) {
						const auto dt = static_cast<real32>(off) * sens;
						const auto new_v = current + dt;

						if (new_v <= bound) {
							break;
						}

						--off;
					}
				}
				else if (off < 0) {
					while (off) {
						const auto dt = static_cast<real32>(off) * sens;
						const auto new_v = current + dt;

						if (new_v >= -bound) {
							break;
						}

						++off;
					}
				}
			};

			adjust_coord(m.offset.x, sensitivity.x, simulated_offset.x, half_bound.x);
			adjust_coord(m.offset.y, sensitivity.y, simulated_offset.y, half_bound.y);

			simulated_offset += static_cast<vec2>(m.offset) * sensitivity;
		}
	}
}
