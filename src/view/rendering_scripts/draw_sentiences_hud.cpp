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

using namespace augs::gui::text;

void draw_sentiences_hud(const draw_sentiences_hud_input in) {
	const auto& cosm = in.cosm;
	const auto& interp = in.interpolation;

	const auto& watched_character = cosm[in.viewed_character_id];
	const auto watched_character_transform = watched_character.get_viewing_transform(interp);
	const auto queried_camera_aabb = in.queried_cone.get_visible_world_rect_aabb();

	auto viewer_faction_matches = [&](const auto f) {
		if (!watched_character) {
			return false;
		}

		return watched_character.get_official_faction() == f.get_official_faction();
	};

	const auto timestamp_ms = static_cast<unsigned>(in.global_time_seconds * 1000);
	const auto outermost_circle_radius = static_cast<int>(in.meters[0].tex.get_original_size().x / 2);

	auto draw_sentience = [&](const auto& v) {
		const auto& sentience = v.template get<components::sentience>();
		const bool is_conscious = sentience.is_conscious();

		const bool is_enemy = !viewer_faction_matches(v);

		if (is_enemy) {
			const auto& danger_indicators = in.settings.draw_danger_indicators;

			if (danger_indicators.is_enabled) {
				const auto show_danger_secs = in.settings.show_danger_indicator_for_seconds;
				const auto fade_danger_secs = in.settings.fade_danger_indicator_for_seconds;

				const auto since_danger = ::secs_since_caused_danger(v);
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

			if (!in.settings.draw_enemy_hud) {
				return;
			}
		}

		const auto transform = v.get_viewing_transform(interp);

		const auto starting_health_angle = [&]() {
			if (v == watched_character) {
				return watched_character_transform.rotation + 135;
			}

			return (v.get_viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
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

						bar_info out;
						const auto& m = sentience.template get<I>();

						out.ratio = m.get_ratio();
						out.value = m.value;

						if constexpr(std::is_same_v<M, health_meter>) {
							out.is_health = true;

							const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - out.ratio));
							const float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

							out.color = sentience.calc_health_color(time_pulse_ratio);
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
				if (v == watched_character) {
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

				const auto health_col = info.color;

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
				};

				std::vector<circle_info> textual_infos;

				auto push_textual_info = [&](const circle_info& i) {
					if (should_draw_textuals) {
						textual_infos.push_back(i);
					}
				};

				if (should_draw_ammo && v == watched_character) {
					const auto examine_item_slot = [&](
						const const_inventory_slot_handle hand,
						const float lower_outside,
						const float max_angular_length,
						const bool ccw
					) {
						if (hand.alive() && hand.has_items()) {
							const auto weapon = cosm[hand.get_items_inside()[0]];
							const auto ammo_info = calc_ammo_info(weapon);

							int total_ammo_for_this_weapon = 0;
							inventory_space_type total_ammo_space_occupied = 0;

							const auto charge_flavour = ::calc_default_charge_flavour(weapon);

							auto traversed_slots = slot_flags();

							for (auto& f : traversed_slots) {
								f = true;
							}

							traversed_slots.set(slot_function::PRIMARY_HAND, false);
							traversed_slots.set(slot_function::SECONDARY_HAND, false);

							watched_character.for_each_contained_item_recursive(
								[&](const auto& typed_item) {
									if (typed_item.get_current_slot().is_hand_slot()) {
										return;
									}

									if (entity_flavour_id(typed_item.get_flavour_id()) == charge_flavour) {
										total_ammo_for_this_weapon += typed_item.template get<components::item>().get_charges();
										total_ammo_space_occupied += *typed_item.find_space_occupied();
									}
								},
								traversed_slots
							);

							if (ammo_info.total_ammo_space > 0) {
								const auto ammo_ratio = ammo_info.get_ammo_ratio();

								auto ammo_color = augs::interp(white, red_violet, (1 - ammo_ratio)* (1 - ammo_ratio));
								ammo_color.a = 200;

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

								if (in.settings.draw_remaining_ammo) {
									const auto remaining_ammo_text = " /" + std::to_string(remaining_ammo);
									new_info.text += formatted_string { remaining_ammo_text, { in.gui_font, remaining_ammo_color } };
								}

								push_textual_info(new_info);
							}
						}
					};

					examine_item_slot(v.get_secondary_hand(), starting_health_angle + 90.f + 22.5f, 45.f, false);
					examine_item_slot(v.get_primary_hand(), starting_health_angle - 22.5f - 45.f, 45.f, true);
				}

				const auto empty_health_amount = static_cast<int>((1 - hr) * 90);

				push_textual_info({
					starting_health_angle + 90 - empty_health_amount / 2,
					formatted_string { std::to_string(int(value) == 0 ? 1 : int(value)), { in.gui_font, health_col } },
					false
				});

				push_textual_info({
					starting_health_angle,
					formatted_string { get_bbcoded_entity_name(v), { in.gui_font, health_col } },
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
					const auto text_pos = screen_space_circle_center + ::position_rectangle_around_a_circle(cam.eye.zoom * (circle_radius + 6.f), bbox, info.angle) - bbox / 2;
					//health_points.pos = screen_space_circle_center + vec2::from_degrees(in.angle).set_length(circle_displacement_length);

					augs::gui::text::print_stroked(
						augs::drawer { info.is_nickname ? in.nicknames : in.health_numbers },
						text_pos,
						text
					);
				}
			}
		}

		const auto& teammate_indicators = in.settings.draw_teammate_indicators;

		const auto show_ko_secs = in.settings.show_death_indicator_for_seconds;
		const auto fade_ko_secs = in.settings.fade_death_indicator_for_seconds;

		const auto since_ko = ::secs_since_knockout(v);
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

			if (in.indicator_meta.bomb_owner == v.get_id()) {
				next_tex = in.bomb_indicator_tex;
			}

			if (in.indicator_meta.now_defusing == v.get_id()) {
				next_tex = in.defusing_indicator_tex;
			}

			std::string primary_text;
			std::string secondary_text;

			{
				const auto& fallback_color = in.indicator_meta.draw_nicknames_for_fallback;

				if (fallback_color.is_enabled) {
					if (col == fallback_color.value) {
						secondary_text = v.get_name();

						const auto max_char_count = static_cast<std::size_t>(in.settings.nickname_characters_for_offscreen_indicators);

						if (secondary_text.size() > max_char_count) {
							secondary_text.resize(max_char_count);
						}

						secondary_text = "(" + secondary_text + ")";
					}
				}
			}

			if (in.settings.draw_offscreen_callouts) {
				if (const auto callout = cosm[::get_current_callout(v, interp)]) {
					primary_text = callout.get_name();
				}
			}

			const bool its_somebody_else = watched_character != v;
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
		cosm.for_each_having<components::sentience>(draw_sentience);
	}
	else {
		auto draw_sentience_callback = [&](const auto& v) {
			v.template dispatch_on_having_all<components::sentience>(
				[&](const auto& typed_sentience) {
					draw_sentience(typed_sentience);
				}
			);
		};

		const auto& visible_entities = in.all;
		visible_entities.for_each<render_layer::SENTIENCES>(cosm, draw_sentience_callback);
	}
}
