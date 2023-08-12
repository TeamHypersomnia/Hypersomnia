#include "rendering_scripts.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/inferred_caches/tree_of_npo_cache.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/components/sentience_component.h"
#include "game/components/container_component.h"
#include "augs/string/string_templates.h"
#include "game/detail/describers.h"
#include "augs/graphics/vertex.h"
#include "augs/gui/text/printer.h"
#include "augs/image/font.h"
#include "augs/math/math.h"
#include "view/game_drawing_settings.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/detail/visible_entities.h"
#include "game/cosmos/for_each_entity.h"
#include "augs/drawing/sprite_helpers.h"
#include "game/detail/sentience/sentience_logic.h"
#include "game/detail/sentience/callout_logic.h"
#include "view/rendering_scripts/draw_offscreen_indicator.h"
#include "augs/drawing/drawing.hpp"
#include "game/detail/calc_ammo_info.hpp"
#include "game/detail/get_hovered_world_entity.h"

#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "augs/log.h"
#include "view/audiovisual_state/systems/damage_indication_system.h"

#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/explosive/like_explosive.h"

using namespace augs::gui::text;

void draw_sentiences_hud(const draw_sentiences_hud_input in) {
	if (in.meters.empty()) {
		return;
	}

	const auto& cosm = in.cosm;
	const auto& interp = in.interpolation;

	const auto& watched_character = cosm[in.viewed_character_id];

	float teleport_alpha = 1.0f;

	if (auto rigid_body = watched_character.find<components::rigid_body>()) {
		teleport_alpha = rigid_body.get_teleport_alpha();
	}

	const auto watched_character_transform = watched_character.get_viewing_transform(interp);
	const auto queried_camera_aabb = in.queried_cone.get_visible_world_rect_aabb();

	auto viewer_faction_matches = [&](const auto f) {
		if (!watched_character) {
			return false;
		}

		return watched_character.get_official_faction() == f.get_official_faction();
	};

	const auto outermost_circle_radius = static_cast<int>(in.meters[0].tex.get_original_size().x / 2);

	auto draw_character = [&](const auto& drawn_character) {
		const auto& sentience = drawn_character.template get<components::sentience>();
		const bool is_conscious = sentience.is_conscious();

		const bool is_enemy = !viewer_faction_matches(drawn_character);
		const auto transform = drawn_character.get_viewing_transform(interp);

		if (is_enemy) {
			const auto& danger_indicators = in.settings.draw_danger_indicators;

			if (danger_indicators.is_enabled) {
				const auto show_danger_secs = in.settings.show_danger_indicator_for_seconds;
				const auto fade_danger_secs = in.settings.fade_danger_indicator_for_seconds;

				const auto since_danger = ::secs_since_caused_danger(drawn_character);
				const bool dangered_recently = since_danger && since_danger <= show_danger_secs;

				if (dangered_recently) {
					auto col = danger_indicators.value;

					if (dangered_recently) {
						if (fade_danger_secs > 0.f) {
							const auto fade_mult = std::clamp((show_danger_secs - *since_danger) / fade_danger_secs, 0.f, 1.f);
							col.mult_alpha(fade_mult);
						}
					}

					const auto danger_pos = sentience.transform_when_danger_caused.pos;
					const auto danger_radius = sentience.radius_of_last_caused_danger;
					const auto distance_to_danger_sq = (watched_character_transform.pos - danger_pos).length_sq() ;

					if (distance_to_danger_sq <= danger_radius * danger_radius) {
						const auto indicator_pos = in.text_camera.to_screen_space(danger_pos);
						const auto& indicator_tex = in.danger_indicator_tex;

						std::string primary_text;

						if (in.settings.draw_offscreen_callouts) {
							if (const auto callout = cosm[::get_current_callout(cosm, danger_pos)]) {
								primary_text = callout.get_name();
							}
						}

						const bool draw_offscreen = in.settings.draw_offscreen_indicators;
						const bool draw_onscreen = true;

						const auto screen_size = in.text_camera.screen_size;

						auto indicators_output = augs::drawer { in.indicators };

						::draw_offscreen_indicator(
							indicators_output,
							draw_offscreen,
							draw_onscreen,
							col,
							indicator_tex,
							indicator_pos,
							screen_size,
							false,
							std::nullopt,
							in.gui_font,
							primary_text,
							{},
							in.text_camera.to_screen_space(watched_character_transform.pos),
							in.settings.offscreen_reference_mode
						);
					}
				}
			}

			switch (in.settings.enemy_hud_mode) {
				case character_hud_type::SMALL_HEALTH_BAR: {
					if (!in.is_reasonably_in_view(drawn_character)) {
						return;
					}

					const auto shield = ::get_shield_ratio(drawn_character);
					const auto health = ::get_health_ratio(drawn_character);

					const bool draw_shield = shield > 0.0f;

					const auto& meter_metas = cosm.get_common_significant().meters;
					const auto chosen_ratio = draw_shield ? shield : health;

					const auto& chosen_appearance = 
						draw_shield ? 
						std::get<personal_electricity_meter>(meter_metas).appearance : 
						std::get<health_meter>(meter_metas).appearance
					;

					auto bar_color = chosen_appearance.bar_color;
					bar_color.mult_alpha(teleport_alpha);

					// brighten up the health bar a little
					auto bright_bar_color = draw_shield ? rgba(bar_color).mult_brightness(1.25f) : bar_color;
					bright_bar_color.mult_alpha(teleport_alpha);

					auto dark_bar_color = rgba(bar_color) * 0.4f;
					// now we have a bit less alpha too
					dark_bar_color.a = 200;
					dark_bar_color.mult_alpha(teleport_alpha);

					const auto bar_center = in.text_camera.to_screen_space(transform.pos + vec2(0, outermost_circle_radius));

					const auto internal_border_size = 0;
					const auto external_border_size = 1;
					const auto internal_bar_size = vec2i(72, 4);

					const auto internal_current_width = is_conscious ? std::max(1, static_cast<int>(static_cast<float>(internal_bar_size.x) * chosen_ratio)) : 0;
					const auto internal_bar_current_size = vec2i(internal_current_width, internal_bar_size.y);

					const int total_border_extent = internal_border_size + external_border_size;

					const auto total_size = internal_bar_size + 2 * vec2i(total_border_extent, total_border_extent);

					auto bars_output = augs::drawer { in.small_health_bars };

					const auto bigger_origin = ltrb::center_and_size(bar_center, total_size);

					const auto smaller_left = bigger_origin.l + total_border_extent;
					const auto smaller_origin = ltrb(smaller_left, bigger_origin.t + total_border_extent, smaller_left + internal_bar_current_size.x, bigger_origin.b - total_border_extent);

					const auto tex = in.small_health_bar_tex;

					const bool draw_partial_borders = true;

					if (is_conscious) {
						bars_output.aabb(tex, bigger_origin, dark_bar_color);
						bars_output.aabb(tex, smaller_origin, bright_bar_color);

						if (draw_partial_borders && chosen_ratio < 1.0f) {
							{
								const auto dim_border_col = rgba(bar_color).mult_brightness(0.5f);
								bars_output.border(in.small_health_bar_tex, bigger_origin, dim_border_col);
							}

							const auto top_border =     ltrb(bigger_origin.l-1, bigger_origin.t-1, smaller_origin.r, bigger_origin.t);
							const auto left_border =    ltrb(bigger_origin.l-1, bigger_origin.t,   bigger_origin.l, bigger_origin.b);
							const auto bottom_border =  ltrb(bigger_origin.l-1, bigger_origin.b,   smaller_origin.r, bigger_origin.b+1);
							//const auto right_border =   ltrb(bigger_origin.r  , bigger_origin.t,   bigger_origin.r+1, bigger_origin.b);

							bars_output.aabb(tex, left_border, bright_bar_color);
							bars_output.aabb(tex, top_border, bright_bar_color);
							bars_output.aabb(tex, bottom_border, bright_bar_color);
							//bars_output.aabb(tex, right_border, bright_bar_color);
						}
						else {
							bars_output.border(in.small_health_bar_tex, bigger_origin, bright_bar_color);
						}
					}

					if (const auto highlight = in.damage_indication_sys.find_white_highlight(drawn_character, in.damage_indication_settings)) {
						const auto white_bar_width_inclusive = static_cast<int>(static_cast<float>(internal_bar_size.x) * highlight->original_ratio);

						const auto white_bar_origin = ltrb(
							smaller_origin.r,
							smaller_origin.t,
							smaller_left + white_bar_width_inclusive,
							smaller_origin.b
						);

						auto white_bar_color = white;
						white_bar_color.mult_alpha(1.0f - highlight->passed_mult);
						white_bar_color.mult_alpha(teleport_alpha);

						bars_output.aabb(tex, white_bar_origin, white_bar_color);

						if (draw_partial_borders) {
							const auto top_border =     ltrb(white_bar_origin.l, bigger_origin.t-1, white_bar_origin.r, bigger_origin.t);
							const auto bottom_border =  ltrb(white_bar_origin.l, bigger_origin.b,   white_bar_origin.r, bigger_origin.b+1);

							bars_output.aabb(tex, top_border, white_bar_color);
							bars_output.aabb(tex, bottom_border, white_bar_color);
						}
					}

					return;
				}
				case character_hud_type::FULL:
					break;
				default:
					return;
			}
		}

		const auto starting_health_angle = [&]() {
			if (drawn_character == watched_character) {
				return watched_character_transform.rotation + 135;
			}

			return (drawn_character.get_viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
		}();

		for (const auto& meter : in.meters) {
			const auto& this_tex = meter.tex;
			const auto circle_radius = static_cast<int>(this_tex.get_original_size().x / 2);
			auto meter_output = augs::drawer { meter.output.triangles };
			auto& meter_specials = meter.output.specials;

			struct bar_info {
				bool is_health = false;
				rgba color;
				real32 ratio;
				real32 value;
			};

			const auto info = [&]() -> bar_info {
				return meter.id.dispatch(
					[&](auto d) {
						using I = decltype(d);
						using M = typename I::meta_type;

						const auto& m = sentience.template get<I>();

						auto out = bar_info();
						out.ratio = std::max(0.0f, m.get_ratio());
						out.value = m.value;

						if constexpr(std::is_same_v<M, health_meter>) {
							out.is_health = true;

							const auto single_pulse_cycle_ms = static_cast<int>(1250 - 1000 * (1 - out.ratio));

							if (single_pulse_cycle_ms > 0) {
								const auto global_time_ms = static_cast<unsigned>(in.global_time_seconds * 1000);
								const float time_pulse_ratio = (global_time_ms % single_pulse_cycle_ms) / static_cast<float>(single_pulse_cycle_ms);

								out.color = sentience.calc_health_color(time_pulse_ratio);
							}
							else {
								out.color = sentience.calc_health_color(1.0f);
							}
						}
						else {
							const auto& meta = std::get<M>(cosm.get_common_significant().meters);

							out.is_health = false;
							out.color = meta.appearance.bar_color;
						}

						return out;
					}
				);
			}();

			const auto hr = info.ratio;

			const auto ending_health_angle = [&]() {
				if (drawn_character == watched_character) {
					return starting_health_angle + hr * 90.f;
				}

				return starting_health_angle + hr * 90.f;
			}();


			const auto bar_aabb = ltrb::center_and_size(transform.pos, this_tex.get_original_size());
			const bool bar_visible = bar_aabb.hover(queried_camera_aabb);

			if (is_conscious && bar_visible) {
				const bool should_draw_textuals = info.is_health;
				const bool should_draw_ammo = info.is_health;

				const auto value = info.value;

				auto health_col = info.color;
				health_col.mult_alpha(teleport_alpha);

				meter_output.aabb_centered(this_tex, vec2(transform.pos).discard_fract(), health_col);

				const auto push_angles = [&](
					const float lower_outside, 
					const float upper_outside, 
					const float lower_inside, 
					const float upper_inside
				) {
					augs::special s;

					s.v1.set(augs::normalize_degrees(lower_outside), augs::normalize_degrees(upper_outside + 0.001f)) /= 180;
					s.v2.set(augs::normalize_degrees(lower_inside), augs::normalize_degrees(upper_inside + 0.001f)) /= 180;

					meter_specials.push_back(s);
					meter_specials.push_back(s);
					meter_specials.push_back(s);
					
					meter_specials.push_back(s);
					meter_specials.push_back(s);
					meter_specials.push_back(s);
				};

				push_angles(starting_health_angle, starting_health_angle + 90, starting_health_angle, ending_health_angle);

				struct circle_info {
					float angle;
					formatted_string text;
					bool is_nickname = false;
					float circle_radius_mult_for_text = 1.f;
				};

				std::vector<circle_info> textual_infos;

				auto push_textual_info = [&](const circle_info& i) {
					if (should_draw_textuals) {
						textual_infos.push_back(i);
					}
				};

				bool drawn_melee_count_once = false;

				if (should_draw_ammo && drawn_character == watched_character) {
					const auto examine_item_slot = [&](
						const const_inventory_slot_handle hand,
						const float lower_outside,
						const float max_angular_length,
						const bool ccw
					) {
						if (hand.alive() && hand.has_items()) {
							const auto weapon = cosm[hand.get_items_inside()[0]];

							if (in.settings.draw_remaining_ammo) {
								auto draw_count = [&](const int count) {
									circle_info new_info;

									auto remaining_ammo_color = white;
									remaining_ammo_color.a = 200;
									remaining_ammo_color.mult_alpha(teleport_alpha);

									const auto remaining_ammo_text = std::to_string(count);
									new_info.text = formatted_string { remaining_ammo_text, { in.gui_font, remaining_ammo_color } };
									new_info.angle = lower_outside;
									new_info.circle_radius_mult_for_text = 0.85f;

									push_textual_info(new_info);
								};

								if (weapon.has<components::melee>() && !drawn_melee_count_once) {
									drawn_melee_count_once = true;
									int count_melees = 0;

									watched_character.for_each_contained_item_recursive(
										[&](const auto& typed_item) {
											if (typed_item.template find<components::melee>()) {
												++count_melees;
											}
										}
									);

									draw_count(count_melees);

									return;
								}

								if (weapon.has<components::hand_fuse>() && !::is_like_plantable_bomb(weapon)) {
									int count_this_kind = 0;

									watched_character.for_each_contained_item_recursive(
										[&](const auto& typed_item) {
											if (typed_item.get_flavour_id() == weapon.get_flavour_id()) {
												++count_this_kind;
											}
										}
									);

									draw_count(count_this_kind);

									return;
								}
							}

							const auto ammo_info = calc_ammo_info(weapon);

							int total_ammo_for_this_weapon = 0;
							inventory_space_type total_ammo_space_occupied = 0;

							const auto ammo_piece_flavour = ::calc_purchasable_ammo_piece(weapon);

							{
								watched_character.for_each_contained_item_recursive(
									[&](const auto& ammo_piece) {
										if (entity_flavour_id(ammo_piece.get_flavour_id()) == weapon.get_flavour_id()) {
											return recursive_callback_result::CONTINUE_DONT_RECURSE;
										}

										if (entity_flavour_id(ammo_piece.get_flavour_id()) == ammo_piece_flavour) {
											auto count_charge_stack = [&](const auto& ammo_stack) {
												if (ammo_stack.template has<invariants::cartridge>()) {
													total_ammo_for_this_weapon += ammo_stack.template get<components::item>().get_charges();
													total_ammo_space_occupied += *ammo_stack.find_space_occupied();
												}
											};

											count_charge_stack(ammo_piece);
											
											ammo_piece.for_each_contained_item_recursive(count_charge_stack);

											return recursive_callback_result::CONTINUE_DONT_RECURSE;
										}

										return recursive_callback_result::CONTINUE_AND_RECURSE;
									},
									std::nullopt
								);
							}

							if (ammo_info.total_ammo_space > 0) {
								const auto ammo_ratio = ammo_info.get_ammo_ratio();

								auto ammo_color = augs::interp(white, red_violet, (1 - ammo_ratio)* (1 - ammo_ratio));
								ammo_color.a = 200;
								ammo_color.mult_alpha(teleport_alpha);

								meter_output.aabb_centered(this_tex, vec2(transform.pos).discard_fract(), ammo_color);

								circle_info new_info;

								auto upper_outside = lower_outside + max_angular_length;

								auto empty_amount = (1 - ammo_ratio) * max_angular_length;

								if (!ccw) {
									push_angles(lower_outside, upper_outside, lower_outside, lower_outside + ammo_ratio * max_angular_length);
									new_info.angle = upper_outside - empty_amount / 2;
								}
								else {
									push_angles(lower_outside, upper_outside, upper_outside - ammo_ratio * max_angular_length, upper_outside);
									new_info.angle = lower_outside + empty_amount / 2;
								}

								const auto current_ammo_text = std::to_string(ammo_info.total_charges);
								new_info.text = formatted_string { current_ammo_text, { in.gui_font, ammo_color } };

								const auto remaining_ammo_space_occupied = total_ammo_space_occupied;
								const auto remaining_ammo = total_ammo_for_this_weapon;
								const auto remaining_ratio = std::min(1.f, float(remaining_ammo_space_occupied) / ammo_info.total_ammo_space);

								auto remaining_ammo_color = augs::interp(white, red_violet, (1 - remaining_ratio)* (1 - remaining_ratio));
								remaining_ammo_color.a = 200;
								remaining_ammo_color.mult_alpha(teleport_alpha);

								if (in.settings.draw_remaining_ammo) {
									const auto remaining_ammo_text = " /" + std::to_string(remaining_ammo);
									new_info.text += formatted_string { remaining_ammo_text, { in.gui_font, remaining_ammo_color } };
								}

								push_textual_info(new_info);
							}
						}
					};

					examine_item_slot(drawn_character.get_primary_hand(), starting_health_angle - 22.5f - 45.f, 45.f, true);
					examine_item_slot(drawn_character.get_secondary_hand(), starting_health_angle + 90.f + 22.5f, 45.f, false);
				}

				const auto empty_health_amount = static_cast<int>((1 - hr) * 90);

				push_textual_info({
					starting_health_angle + 90 - empty_health_amount / 2,
					formatted_string { std::to_string(int(value) == 0 ? 1 : int(value)), { in.gui_font, health_col } },
					false
				});

				push_textual_info({
					starting_health_angle,
					formatted_string { get_bbcoded_entity_name(drawn_character), { in.gui_font, health_col } },
					true
				});

				for (const auto& info : textual_infos) {
					if (info.text.empty()) {
						continue;
					}

					//const auto circle_displacement_length = health_points.get_bbox().bigger_side() + circle_radius;
					const auto& text = info.text;
					const auto bbox = get_text_bbox(text);
					
					const auto cam = in.text_camera;
					const vec2i screen_space_circle_center = cam.to_screen_space(transform.pos);
					const auto radius = circle_radius * info.circle_radius_mult_for_text;
					const auto text_pos = screen_space_circle_center + ::position_rectangle_around_a_circle(cam.eye.zoom * (radius + 6.f), bbox, info.angle) - bbox / 2;
					//health_points.pos = screen_space_circle_center + vec2::from_degrees(in.angle).set_length(circle_displacement_length);

					auto stroke_color = black;
					stroke_color.mult_alpha(teleport_alpha);

					augs::gui::text::print_stroked(
						augs::drawer { info.is_nickname ? in.nicknames : in.health_numbers },
						text_pos,
						text,
						{},
						stroke_color
					);
				}
			}
		}

		const auto& teammate_indicators = in.settings.draw_teammate_indicators;

		const auto show_ko_secs = in.settings.show_death_indicator_for_seconds;
		const auto fade_ko_secs = in.settings.fade_death_indicator_for_seconds;

		const auto since_ko = ::secs_since_knockout(drawn_character);
		const bool koed_recently = since_ko && since_ko <= show_ko_secs;

		const auto indicator_tex = is_conscious ? in.color_indicator_tex : in.death_indicator_tex;
		const bool should_rotate_indicator_tex = is_conscious;

		const bool indicators_enabled = teammate_indicators.is_enabled && teammate_indicators.value > 0.f;

		auto col = sentience.last_assigned_color;

		if (indicators_enabled && (is_conscious || koed_recently) && col.a > 0) {
			const auto cam = in.text_camera;
			const vec2i screen_space_circle_center = cam.to_screen_space(transform.pos);
			const auto angle = starting_health_angle + in.color_indicator_angle;
			const auto bbox = indicator_tex.get_original_size();

			const auto indicator_pos = screen_space_circle_center + ::position_rectangle_around_a_circle(cam.eye.zoom * (outermost_circle_radius + 6.f), bbox, angle) - bbox / 2;

			col.mult_alpha(teammate_indicators.value);

			if (koed_recently) {
				if (fade_ko_secs > 0.f) {
					const auto fade_mult = std::clamp((show_ko_secs - *since_ko) / fade_ko_secs, 0.f, 1.f);
					col.mult_alpha(fade_mult);
				}
			}

			std::optional<augs::atlas_entry> next_tex;

			if (in.indicator_meta.bomb_owner == drawn_character.get_id()) {
				next_tex = in.bomb_indicator_tex;
			}

			if (in.indicator_meta.now_defusing == drawn_character.get_id()) {
				next_tex = in.defusing_indicator_tex;
			}

			std::string primary_text;
			std::string secondary_text;

			{
				const auto& fallback_color = in.indicator_meta.draw_nicknames_for_fallback;

				if (fallback_color.is_enabled) {
					if (col == fallback_color.value) {
						secondary_text = drawn_character.get_name();

						const auto max_char_count = static_cast<std::size_t>(in.settings.nickname_characters_for_offscreen_indicators);

						if (secondary_text.size() > max_char_count) {
							secondary_text.resize(max_char_count);
						}

						secondary_text = "(" + secondary_text + ")";
					}
				}
			}

			if (in.settings.draw_offscreen_callouts) {
				if (const auto callout = cosm[::get_current_callout(drawn_character, interp)]) {
					primary_text = callout.get_name();
				}
			}

			const bool its_somebody_else = watched_character != drawn_character;
			const bool draw_offscreen = its_somebody_else && in.settings.draw_offscreen_indicators;
			const bool draw_onscreen = true;

			const auto screen_size = cam.screen_size;

			auto indicators_output = augs::drawer { in.indicators };

			::draw_offscreen_indicator(
				indicators_output,
				draw_offscreen,
				draw_onscreen,
				col,
				indicator_tex,
				indicator_pos,
				screen_size,
				should_rotate_indicator_tex,
				next_tex,
				in.gui_font,
				primary_text,
				secondary_text,
				in.text_camera.to_screen_space(watched_character_transform.pos),
				in.settings.offscreen_reference_mode
			);
		}
	};

	if (in.settings.draw_offscreen_indicators) {
		cosm.for_each_having<components::sentience>(draw_character);
	}
	else {
		auto draw_sentience_callback = [&](const auto& drawn_character) {
			drawn_character.template dispatch_on_having_all<components::sentience>(
				[&](const auto& typed_sentience) {
					draw_character(typed_sentience);
				}
			);
		};

		const auto& visible_entities = in.all;
		visible_entities.for_each<render_layer::SENTIENCES>(cosm, draw_sentience_callback);
	}
}
