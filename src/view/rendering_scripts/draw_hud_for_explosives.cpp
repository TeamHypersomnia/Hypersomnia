#include "rendering_scripts.h"
#include "augs/drawing/drawing.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sprite_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/components/fixtures_component.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/detail/hand_fuse_math.h"
#include "game/detail/bombsite_in_range.h"

void draw_hud_for_explosives(const draw_hud_for_explosives_input in) {
	const auto dt = in.cosm.get_fixed_delta();

	const auto& cosmos = in.cosm;

	cosmos.for_each_having<components::hand_fuse>(
		[&](const auto it) {
			const auto& fuse = it.template get<components::hand_fuse>();
			const auto& fuse_def = it.template get<invariants::hand_fuse>();

			const auto tex = in.circular_bar_tex;

			auto draw_circle = [&](const auto highlight_amount, const rgba first_col, const rgba second_col) {
				if (highlight_amount >= 0.f && highlight_amount <= 1.f) {
					const auto highlight_color = augs::interp(first_col, second_col, (1 - highlight_amount)* (1 - highlight_amount));

					if (const auto tr = it.find_viewing_transform(in.interpolation)) {
						in.output.aabb_centered(tex, tr->pos, highlight_color);

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
			};

			const auto t = in.only_type;

			if (t == circular_bar_type::MEDIUM && fuse_def.has_delayed_arming()) {
				if (fuse.arming_requested) {
					auto first_col = white;
					auto second_col = red_violet;

					if (!::bombsite_in_range(it)) {
						first_col = red;
						second_col = red;
					}

					const auto when_started_arming = 
						fuse.when_started_arming.was_set() ? 
						fuse.when_started_arming.in_seconds(dt) :
						in.global_time_seconds
					;

					const auto highlight_amount = static_cast<float>(
						(in.global_time_seconds - when_started_arming)
						/ (fuse_def.arming_duration_ms / 1000.f) 
					);

					draw_circle(highlight_amount, first_col, second_col);
				}
			}

			if (const auto when_armed = fuse.when_armed; when_armed.was_set()) {
				const auto highlight_amount = static_cast<float>(1 - (
					(in.global_time_seconds - when_armed.in_seconds(dt))
					/ (fuse_def.fuse_delay_ms / 1000.f) 
				));

				if (t == circular_bar_type::MEDIUM && fuse_def.has_delayed_arming()) {
					draw_circle(highlight_amount, white, red_violet);
				}
				else if (t == circular_bar_type::SMALL && !fuse_def.has_delayed_arming()) {
					draw_circle(highlight_amount, white, red_violet);
				}
			}

			if (t == circular_bar_type::OVER_MEDIUM && fuse_def.defusing_enabled()) {
				if (const auto amount_defused = fuse.amount_defused; amount_defused >= 0.f) {
					const auto highlight_amount = static_cast<float>(
						amount_defused / fuse_def.defusing_duration_ms
					);

					draw_circle(highlight_amount, white, red_violet);
				}
			}
		}
	);
}

void draw_beep_lights::operator()() {
	const auto clk = cosm.get_clock();

	cosm.for_each_having<components::hand_fuse>(
		[&](const auto it) {
			const auto& fuse = it.template get<components::hand_fuse>();

			if (fuse.armed()) {
				const auto& fuse_def = it.template get<invariants::hand_fuse>();
				auto beep_col = fuse_def.beep_color;

				if (beep_col.a) {
					const auto beep = beep_math { fuse, fuse_def, clk };

					if (const auto mult = beep.get_beep_light_mult(); mult > AUGS_EPSILON<real32>) {
						beep_col.multiply_alpha(mult);
						if (const auto tr = it.find_viewing_transform(interpolation)) {
							output.aabb_centered(
								cast_highlight_tex,
								tr->pos,
								beep_col
							);
						}
					}
				}
			}
		}
	);
}
