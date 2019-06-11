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

using namespace augs::gui::text;

draw_sentiences_hud_output draw_sentiences_hud(const draw_sentiences_hud_input in) {
	const auto& visible_entities = in.all;
	const auto& cosm = in.cosm;
	const auto& interp = in.interpolation;
	const auto output = in.output;
	auto& specials = in.specials;

	const auto& watched_character = cosm[in.viewed_character_id];

	auto viewer_faction_matches = [&](const auto f) {
		if (!watched_character) {
			return false;
		}

		return watched_character.get_official_faction() == f.get_official_faction();
	};

	const auto timestamp_ms = static_cast<unsigned>(in.global_time_seconds * 1000);
	const auto circle_radius = static_cast<int>(in.circular_bar_tex.get_original_size().x / 2);

	draw_sentiences_hud_output out;

	auto draw_sentience = [&](const auto& v) {
		if (!in.settings.draw_enemy_hud && !viewer_faction_matches(v)) {
			return;
		}

		if (const auto* const sentience = v.template find<components::sentience>()) {
			struct bar_info {
				bool is_health = false;
				rgba color;
				real32 ratio;
				real32 value;
			};

			const auto info = [&]() -> bar_info {
				return in.meter.dispatch(
					[&](auto d) {
						using I = decltype(d);
						using M = typename I::meta_type;

						bar_info out;
						const auto& m = sentience->template get<I>();

						out.ratio = m.get_ratio();
						out.value = m.value;

						if constexpr(std::is_same_v<M, health_meter>) {
							out.is_health = true;

							const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - out.ratio));
							const float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

							out.color = sentience->calc_health_color(time_pulse_ratio);
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

			float starting_health_angle = 0.f;
			float ending_health_angle = 0.f;

			const auto watched_character_transform = watched_character.get_viewing_transform(interp);

			if (v == watched_character) {
				starting_health_angle = watched_character_transform.rotation + 135;
				ending_health_angle = starting_health_angle + hr * 90.f;
			}
			else {
				starting_health_angle = (v.get_viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
				ending_health_angle = starting_health_angle + hr * 90.f;
			}

			const auto transform = v.get_viewing_transform(interp);

			const bool is_conscious = sentience->is_conscious();

			if (is_conscious) {
				const bool should_draw_textuals = info.is_health;
				const bool should_draw_ammo = info.is_health;

				const auto value = info.value;

				const auto health_col = info.color;

				output.aabb_centered(in.circular_bar_tex, vec2(transform.pos).discard_fract(), health_col);

				const auto push_angles = [&](
					const float lower_outside, 
					const float upper_outside, 
					const float lower_inside, 
					const float upper_inside
				) {
					augs::special s;

					s.v1.set(augs::normalize_degrees(lower_outside), augs::normalize_degrees(upper_outside + 0.001f)) /= 180;
					s.v2.set(augs::normalize_degrees(lower_inside), augs::normalize_degrees(upper_inside + 0.001f)) /= 180;

					specials.push_back(s);
					specials.push_back(s);
					specials.push_back(s);
					
					specials.push_back(s);
					specials.push_back(s);
					specials.push_back(s);
				};

				push_angles(starting_health_angle, starting_health_angle + 90, starting_health_angle, ending_health_angle);

				struct circle_info {
					float angle;
					std::string text;
					rgba color;
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
						const const_inventory_slot_handle id,
						const float lower_outside,
						const float max_angular_length,
						const bool ccw
					) {
						if (id.alive() && id.has_items()) {
							const auto item = cosm[id.get_items_inside()[0]];

							const auto ammo_info = calc_ammo_info(item);

							if (ammo_info.total_ammo_space > 0) {
								const auto ammo_ratio = ammo_info.get_ammo_ratio();

								auto ammo_color = augs::interp(white, red_violet, (1 - ammo_ratio)* (1 - ammo_ratio));
								ammo_color.a = 200;

								output.aabb_centered(in.circular_bar_tex, vec2(transform.pos).discard_fract(), ammo_color);

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

								new_info.text = std::to_string(ammo_info.total_charges);
								new_info.color = ammo_color;

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
					std::to_string(int(value) == 0 ? 1 : int(value)),
					health_col,
					false
				});

				push_textual_info({
					starting_health_angle,
					get_bbcoded_entity_name(v),
					health_col,
					true
				});

				for (const auto& info : textual_infos) {
					if (info.text.empty()) {
						continue;
					}

					//const auto circle_displacement_length = health_points.get_bbox().bigger_side() + circle_radius;
					const auto text = formatted_string { info.text,{ in.gui_font, info.color } };
					const auto bbox = get_text_bbox(text);
					
					const auto cam = in.text_camera;
					const vec2i screen_space_circle_center = cam.to_screen_space(transform.pos);
					const auto text_pos = screen_space_circle_center + ::position_rectangle_around_a_circle(cam.eye.zoom * (circle_radius + 6.f), bbox, info.angle) - bbox / 2;
					//health_points.pos = screen_space_circle_center + vec2::from_degrees(in.angle).set_length(circle_displacement_length);

					augs::gui::text::print_stroked(
						augs::drawer { info.is_nickname ? out.nicknames : out.health_numbers },
						text_pos,
						text
					);
				}
			}

			const auto& inds = in.draw_color_indicators;

			const auto show_ko_secs = in.settings.show_death_indicator_for_seconds;
			const auto fade_ko_secs = in.settings.fade_death_indicator_for_seconds;

			const auto since_ko = ::secs_since_knockout(v);
			const bool koed_recently = since_ko && since_ko <= show_ko_secs;

			const auto indicator_tex = is_conscious ? in.color_indicator_tex : in.death_indicator_tex;
			const bool should_rotate_indicator_tex = is_conscious;

			const bool indicators_enabled = inds.is_enabled && inds.value > 0.f;

			if (indicators_enabled && (is_conscious || koed_recently)) {
				const auto bbox = indicator_tex.get_original_size();
				
				const auto cam = in.text_camera;
				const vec2i screen_space_circle_center = cam.to_screen_space(transform.pos);
				const auto angle = starting_health_angle + in.color_indicator_angle;
				const auto indicator_pos = screen_space_circle_center + ::position_rectangle_around_a_circle(cam.eye.zoom * (circle_radius + 6.f), bbox, angle) - bbox / 2;

				auto col = sentience->last_assigned_color;
				col.mult_alpha(inds.value);

				if (koed_recently) {
					if (fade_ko_secs > 0.f) {
						const auto fade_mult = std::clamp((show_ko_secs - *since_ko) / fade_ko_secs, 0.f, 1.f);
						col.mult_alpha(fade_mult);
					}
				}

				const auto screen_size = cam.screen_size;
				auto raycasted_aabb = ltrb(vec2::zero, screen_size);
				raycasted_aabb.r -= bbox.x;
				raycasted_aabb.b -= bbox.y;

				const bool is_offscreen = !raycasted_aabb.hover(indicator_pos);

				if (in.draw_offscreen_indicators && is_offscreen) {
					const real32 indicator_angles[4] = {
						-180,
						-90,
						0,
						90
					};

					std::optional<augs::atlas_entry> next_tex;

					if (in.indicator_meta.bomb_owner == v.get_id()) {
						next_tex = in.bomb_indicator_tex;
					}


					const auto raycast_a = indicator_pos;
					const auto raycast_b = raycasted_aabb.get_center();

					const auto edges = raycasted_aabb.make_edges();

					for (std::size_t e = 0; e < edges.size(); ++e) {
						const auto intersection = segment_segment_intersection(raycast_a, raycast_b, edges[e][0], edges[e][1]);

						if (intersection.hit) {
							const auto hit_location = intersection.intersection;
							const auto final_indicator_location = hit_location + bbox / 2;

							auto indicator_angle = 0.f;

							if (should_rotate_indicator_tex) {
								indicator_angle = indicator_angles[e];
							}

							auto indicator_aabb = ltrb(hit_location, bbox);

							augs::detail_sprite(out.color_indicators, indicator_tex, final_indicator_location, indicator_angle, col);

							if (next_tex != std::nullopt) {
								const auto bbox_a = vec2(bbox);
								const auto bbox_b = vec2(next_tex->get_original_size());

								const auto x_off = bbox_b.x / 2 + bbox_a.x / 2;
								const auto y_off = bbox_b.y / 2 + bbox_a.y / 2;

								const vec2 additional_icon_offsets[4] = {
									vec2(0, y_off),
									vec2(-x_off, 0),
									vec2(0, -y_off),
									vec2(x_off, 0)
								};

								const auto next_indicator_location = vec2(final_indicator_location + additional_icon_offsets[e]);

								augs::detail_sprite(out.color_indicators, *next_tex, next_indicator_location, 0, col);

								indicator_aabb.contain(ltrb::center_and_size(next_indicator_location, next_tex->get_original_size()));
							}

							if (in.settings.draw_offscreen_callouts) {
								if (const auto callout = cosm[::get_current_callout(v, interp)]) {
									augs::ralign_flags flags;
									auto text_pos = indicator_pos;

									if (e == 0) {
										if (indicator_pos.x < screen_size.x / 2) {
											text_pos = indicator_aabb.right_top();
										}
										else {
											text_pos = indicator_aabb.left_top();
											flags.set(augs::ralign::R);
										}
									}

									if (e == 2) {
										flags.set(augs::ralign::B);

										if (indicator_pos.x < screen_size.x / 2) {
											text_pos = indicator_aabb.right_bottom();
										}
										else {
											text_pos = indicator_aabb.left_bottom();
											flags.set(augs::ralign::R);
										}
									}

									if (e == 1) {
										text_pos.set(indicator_aabb.l, indicator_aabb.get_center().y);
										flags.set(augs::ralign::R);
										flags.set(augs::ralign::CY);
									}
									if (e == 3) {
										text_pos.set(indicator_aabb.r, indicator_aabb.get_center().y);
										flags.set(augs::ralign::CY);
									}

									const auto text = formatted_string { callout.get_name(), { in.gui_font, col } };

									augs::gui::text::print_stroked(
										augs::drawer { out.color_indicators },
										text_pos,
										text,
										flags
									);
								}
							}

							break;
						}
					}
				}
				else {
					augs::drawer { out.color_indicators }.aabb_lt(indicator_tex, indicator_pos, col);
				}
			}
		}
	};

	if (in.draw_offscreen_indicators) {
		cosm.for_each_having<components::sentience>(draw_sentience);
	}
	else {
		visible_entities.for_each<render_layer::SENTIENCES>(cosm, draw_sentience);
	}

	return out;
}
