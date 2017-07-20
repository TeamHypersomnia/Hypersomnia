#include "all.h"
#include "augs/graphics/drawers.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sprite_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/fixtures_component.h"

namespace rendering_scripts {
	void draw_hud_for_released_explosives(
		augs::vertex_triangle_buffer& in,
		augs::special_buffer& in_special,
		const interpolation_system& sys,
		const camera_cone cam,
		const cosmos& cosm,
		const double global_time_seconds
	) {
		const auto dt = cosm.get_fixed_delta();
		const auto& manager = get_assets_manager();

		cosm.for_each(
			processing_subjects::WITH_HAND_FUSE,
			[&](const auto it) {
				const components::hand_fuse& hand_fuse = it.get<components::hand_fuse>();

				if (!it.get<components::fixtures>().is_activated()) {
					return;
				}

				if (hand_fuse.when_detonates.was_set()) {
					const auto highlight_amount = 1.f - (
						(global_time_seconds - hand_fuse.when_released.in_seconds(dt))
						/
						(hand_fuse.when_detonates.in_seconds(dt) - hand_fuse.when_released.in_seconds(dt))
					);

					if (highlight_amount > 0.f) {
						components::sprite::drawing_input highlight(in);
						highlight.camera = cam;
						highlight.renderable_transform.pos = it.get_viewing_transform(sys).pos;

						const auto col = augs::interp(white, red_violet, (1 - highlight_amount)* (1 - highlight_amount));

						components::sprite spr;

						spr.set(
							assets::game_image_id::CIRCULAR_BAR_SMALL, 
							manager.at(assets::game_image_id::CIRCULAR_BAR_SMALL).get_size(),
							col
						);

						spr.draw(highlight);

						augs::special s;

						const auto full_rot = 360;
						const auto empty_angular_amount = full_rot * (1 - highlight_amount);

						s.v1.set(-90, -90 + full_rot) /= 180;
						s.v2.set(-90 + empty_angular_amount, -90) /= 180;

						in_special.push_back(s);
						in_special.push_back(s);
						in_special.push_back(s);

						in_special.push_back(s);
						in_special.push_back(s);
						in_special.push_back(s);
					}
				}
			}
		);
	}
}
