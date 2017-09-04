#include "rendering_scripts.h"
#include "augs/drawing/drawing.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sprite_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/fixtures_component.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void draw_hud_for_released_explosives(const draw_hud_for_released_explosives_input in) {
	const auto dt = in.cosm.get_fixed_delta();

	in.cosm.for_each(
		processing_subjects::WITH_HAND_FUSE,
		[&](const auto it) {
			const components::hand_fuse& hand_fuse = it.get<components::hand_fuse>();

			if (!it.get<components::fixtures>().is_activated()) {
				return;
			}

			if (hand_fuse.when_detonates.was_set()) {
				const auto highlight_amount = 1.f - (
					(in.global_time_seconds - hand_fuse.when_released.in_seconds(dt))
					/ (hand_fuse.when_detonates.in_seconds(dt) - hand_fuse.when_released.in_seconds(dt))
				);

				if (highlight_amount > 0.f) {
					const auto highlight_color = augs::interp(white, red_violet, (1 - highlight_amount)* (1 - highlight_amount));
					
					in.output.aabb_centered(in.circular_bar_tex, in.camera[it.get_viewing_transform(in.interpolation).pos], highlight_color);

					augs::special s;

					const auto full_rot = 360;
					const auto empty_angular_amount = full_rot * (1 - highlight_amount);

					s.v1.set(-90, -90 + full_rot) /= 180;
					s.v2.set(-90 + empty_angular_amount, -90) /= 180;

					in.specials.push_back(s);
					in.specials.push_back(s);
					in.specials.push_back(s);

					in.specials.push_back(s);
					in.specials.push_back(s);
					in.specials.push_back(s);
				}
			}
		}
	);
}
