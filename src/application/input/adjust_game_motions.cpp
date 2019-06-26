#include "application/input/adjust_game_motions.h"

raw_game_motion to_game_motion(
	raw_game_motion m,
	const vec2 simulated_offset,
	const vec2& sensitivity,
	const vec2i screen_size
) {
	const auto half_bound = vec2(screen_size);

	raw_game_motion out;
	out.motion = m.motion;

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

		out.offset = m.offset;
	}
	else {
		ensure(false && "Unknown (unimplemented) motion type");
	}

	return out;
}
