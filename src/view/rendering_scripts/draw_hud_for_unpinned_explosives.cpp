#include "rendering_scripts.h"
#include "augs/drawing/drawing.h"
#include "game/cosmos/cosmos.h"
#include "game/components/sprite_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/fixtures_component.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void draw_hud_for_unpinned_explosives(const draw_hud_for_released_explosives_input in) {
	const auto dt = in.cosm.get_fixed_delta();

	const auto& cosmos = in.cosm;

	cosmos.for_each_having<components::hand_fuse>(
		[&](const auto it) {
			const components::hand_fuse& hand_fuse = it.template get<components::hand_fuse>();
			const invariants::hand_fuse& hand_fuse_def = it.template get<invariants::hand_fuse>();

			const auto when_unpinned = hand_fuse.when_unpinned;

			if (when_unpinned.was_set()) {
				const auto highlight_amount = static_cast<float>(1 - (
					(in.global_time_seconds - when_unpinned.in_seconds(dt))
					/ (hand_fuse_def.fuse_delay_ms / 1000.f) 
				));

				if (highlight_amount > 0.f) {
					const auto highlight_color = augs::interp(white, red_violet, (1 - highlight_amount)* (1 - highlight_amount));
					
					if (const auto tr = it.find_viewing_transform(in.interpolation)) {
						in.output.aabb_centered(in.circular_bar_tex, tr->pos, highlight_color);

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
		}
	);
}
