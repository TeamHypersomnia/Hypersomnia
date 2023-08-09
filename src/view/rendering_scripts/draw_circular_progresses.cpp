#include "rendering_scripts.h"
#include "augs/drawing/drawing.hpp"
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
#include "view/game_drawing_settings.h"
#include "game/detail/gun/shell_offset.h"
#include "game/detail/gun/gun_cooldowns.h"
#include "augs/log.h"

void draw_circular_progresses(const draw_circular_progresses_input in) {
	using C = circular_bar_type;

	const auto dt = in.cosm.get_fixed_delta();
	const auto& cosm = in.cosm;
	const auto& requests = in.requests;
	const auto clk = cosm.get_clock();
	const auto global_time_seconds = in.global_time_seconds;

	auto get_drawer = [&](const auto& reh) {
		return augs::drawer { reh.output->triangles };
	};

	auto draw_circle_at = [&](const auto& reh_id, const auto& tr, const auto highlight_amount, const rgba first_col, const rgba second_col) {
		if (highlight_amount >= 0.f && highlight_amount <= 1.f) {
			const auto& reh = requests[reh_id];
			const auto& tex = reh.tex;
			const auto highlight_color = augs::interp(first_col, second_col, (1 - highlight_amount)* (1 - highlight_amount));

			auto drawer = get_drawer(reh);
			drawer.aabb_centered(tex, vec2(tr.pos).discard_fract(), highlight_color);

			augs::special s;

			const auto full_rot = 360;
			const auto empty_angular_amount = full_rot * (1 - highlight_amount);

			s.v1.set(0.f, 0.f);
			s.v2.set(-90 + empty_angular_amount, -90) /= 180;

			auto& specials = reh.output->specials;

			specials.push_back(s);
			specials.push_back(s);
			specials.push_back(s);

			specials.push_back(s);
			specials.push_back(s);
			specials.push_back(s);
		}
	};

	auto draw_circle = [&](const auto& reh_id, const auto& it, const auto highlight_amount, const rgba first_col, const rgba second_col, const transformr offset = transformr()) {
		if (auto tr = it.find_viewing_transform(in.interpolation)) {
			draw_circle_at(reh_id, *tr * offset, highlight_amount, first_col, second_col);
		}
	};

	const auto& watched_character = cosm[in.viewed_character_id];

	auto viewer_faction_matches = [&](const auto f) {
		if (!watched_character) {
			return false;
		}

		return watched_character.get_official_faction() == f.get_official_faction();
	};

	cosm.for_each_having<components::hand_fuse>(
		[&](const auto& it) {
			const auto& fuse = it.template get<components::hand_fuse>();
			const auto& fuse_def = it.template get<invariants::hand_fuse>();

			auto do_draw_circle = [&](const auto& reh_id, auto&&... args) {
				draw_circle(reh_id, it, args...);
			};

			if (fuse_def.has_delayed_arming()) {
				if (fuse.arming_requested) {
					if (const auto slot = it.get_current_slot()) {
						if (viewer_faction_matches(slot.get_container())) {
							auto first_col = white;
							auto second_col = red_violet;

							if (!::bombsite_in_range(it)) {
								first_col = red;
								second_col = red;
							}

							const auto when_started_arming = 
								fuse.when_started_arming.was_set() ? 
								fuse.when_started_arming.in_seconds(dt) :
								global_time_seconds
							;

							const auto highlight_amount = static_cast<float>(
								(global_time_seconds - when_started_arming)
								/ (fuse_def.arming_duration_ms / 1000.f) 
							);

							do_draw_circle(C::MEDIUM, highlight_amount, first_col, second_col);
						}
					}
				}
			}

			if (fuse.armed()) {
				const auto highlight_amount = static_cast<float>(1 - (
					(global_time_seconds - fuse.when_armed.in_seconds(dt))
					/ (fuse.fuse_delay_ms / 1000.f) 
				));

				if (fuse_def.has_delayed_arming()) {
					do_draw_circle(C::MEDIUM, highlight_amount, white, red_violet);
				}
				else if (!fuse_def.has_delayed_arming()) {
					do_draw_circle(C::SMALL, highlight_amount, white, red_violet);
				}

				if (fuse_def.defusing_enabled()) {
					if (const auto amount_defused = fuse.amount_defused; amount_defused >= 0.f) {
						if (const auto defusing_character = cosm[fuse.character_now_defusing]) {
							if (viewer_faction_matches(defusing_character)) {
								const auto highlight_amount = static_cast<float>(
									amount_defused / fuse_def.defusing_duration_ms
								);

								do_draw_circle(C::OVER_MEDIUM, highlight_amount, white, red_violet);
							}
						}
					}
				}
			}
		}
	);

	const auto enemy_hud = in.settings.enemy_hud_mode;

	auto calc_appropriate_offset = [&](const auto& container) {
		if (container.template find<components::gun>()) {
			const auto offset = ::calc_shell_offset(container);
			return transformr(offset.pos, offset.rotation);
		}

		return transformr();
	};

	cosm.for_each_having<components::gun>(
		[&](const auto& it) {
			if (const auto tr = it.find_viewing_transform(in.interpolation)) {
				const auto& gun = it.template get<components::gun>();

				auto draw_progress = [&](const auto& amount) {
					draw_circle(C::SMALL, it, amount, white, red_violet, calc_appropriate_offset(it));
				};

				const auto& chambering_progress = gun.chambering_progress_ms;
				const auto chambering_duration = ::calc_current_chambering_duration(it);

				if (chambering_progress > 0.f && augs::is_positive_epsilon(chambering_duration)) {
					if (enemy_hud != character_hud_type::FULL) { 
						if (const auto c = it.get_owning_transfer_capability()) {
							if (!viewer_faction_matches(c)) {
								return;
							}
						}
					}

					draw_progress(chambering_progress / chambering_duration);
				}
				else {
					const auto& gun_def = it.template get<invariants::gun>();

					auto is_wielded = [&]() {
						if (const auto slot = it.get_current_slot(); slot && slot.is_hand_slot()) {
							return true;
						}

						return false;
					};

					{
						const auto cooldown = gun_def.shot_cooldown_ms;

						if (cooldown > 1000.f && is_wielded()) {
							const auto r = clk.get_ratio_of_remaining_time(cooldown, gun.fire_cooldown_object);
							const auto transfer_r = clk.get_ratio_of_remaining_time(
								gun_def.get_transfer_shot_cooldown(), 
								it.when_last_transferred()
							);

							const auto later_r = std::max(r, transfer_r);

							if (augs::is_positive_epsilon(later_r)) {
								draw_progress(1.f - later_r);
							}
						}
					}

					const auto chamber_slot = it[slot_function::GUN_CHAMBER];
					const auto chamber_requirement_fulfilled = chamber_slot.dead() || chamber_slot.get_items_inside().size() > 0;

					const auto heat_to_shoot = gun_def.minimum_heat_to_shoot;

					if (heat_to_shoot > 0.f && chamber_requirement_fulfilled && is_wielded()) {
						const auto current_heat = gun.current_heat;

						if (augs::is_positive_epsilon(current_heat) && current_heat <= heat_to_shoot) {
							draw_progress(current_heat / heat_to_shoot);
						}
					}
				}
			}
		}
	);

	const auto& global = cosm.get_global_solvable();

	for (const auto& m : global.pending_item_mounts) {
		const auto& item = cosm[m.first];

		if (item.dead()) {
			continue;
		}

		const auto& request = m.second;

		const auto& progress = request.progress_ms;

		if (progress > 0.f) {
			const auto highlight_amount = 1.f - (progress / request.get_mounting_duration_ms(item));

			if (enemy_hud != character_hud_type::FULL) { 
				if (const auto c = item.get_owning_transfer_capability()) {
					if (!viewer_faction_matches(c)) {
						return;
					}
				}
			}

			if (!request.is_unmounting(item)) {
				if (const auto slot = cosm[request.target]) {
					const auto container = slot.get_container();
					draw_circle(C::SMALL, container, highlight_amount, white, red_violet, calc_appropriate_offset(container));
				}
			}
			else {
				const auto container = item.get_current_slot().get_container();
				draw_circle(C::SMALL, item, highlight_amount, white, red_violet, calc_appropriate_offset(container));
			}
		}
	}
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
						beep_col.mult_alpha(mult);
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
