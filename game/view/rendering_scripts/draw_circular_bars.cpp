#include "all.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_step.h"
#include "game/view/viewing_session.h"
#include "game/systems_inferred/dynamic_tree_system.h"
#include "game/systems_audiovisual/interpolation_system.h"
#include "game/components/sentience_component.h"
#include "game/components/container_component.h"
#include "augs/templates/string_templates.h"
#include "game/detail/entity_description.h"

namespace rendering_scripts {
	augs::vertex_triangle_buffer draw_circular_bars_and_get_textual_info(const viewing_step r) {
		const auto& dynamic_tree = r.cosm.systems_inferred.get<dynamic_tree_system>();
		const auto& visible_entities = r.visible.all;
		auto& target = r.renderer;
		const auto& cosmos = r.cosm;
		const auto& interp = r.session.systems_audiovisual.get<interpolation_system>();

		const auto& watched_character = cosmos[r.viewed_character];

		const auto timestamp_ms = static_cast<unsigned>(r.get_interpolated_total_time_passed_in_seconds() * 1000);

		augs::vertex_triangle_buffer circular_bars_information;

		for (const auto v_id : visible_entities) {
			const auto v = cosmos[v_id];
			const auto* const sentience = v.find<components::sentience>();

			if (sentience && sentience->health.is_enabled()) {
				const auto hr = sentience->health.get_ratio();
				const auto one_less_hr = 1 - hr;

				const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));
				const float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

				const auto health_col = sentience->calculate_health_color(time_pulse_ratio);

				const auto transform = v.get_viewing_transform(interp);

				components::sprite::drawing_input state(r.renderer.triangles);
				state.camera = r.camera;
				state.renderable_transform = transform;
				state.renderable_transform.rotation = 0;

				components::sprite circle_hud;
				circle_hud.set(assets::game_image_id::HUD_CIRCULAR_BAR_MEDIUM, health_col);
				circle_hud.draw(state);

				const auto watched_character_transform = watched_character.get_viewing_transform(r.session.systems_audiovisual.get<interpolation_system>());
				float starting_health_angle = 0.f;
				float ending_health_angle = 0.f;

				if (v == watched_character) {
					starting_health_angle = watched_character_transform.rotation + 135;
					ending_health_angle = starting_health_angle + sentience->health.get_ratio() * 90.f;
				}
				else {
					starting_health_angle = (v.get_viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
					ending_health_angle = starting_health_angle + sentience->health.get_ratio() * 90.f;
				}

				const auto push_angles = [&target](
					const float lower_outside, 
					const float upper_outside, 
					const float lower_inside, 
					const float upper_inside
				) {
					augs::special s;

					s.v1.set(augs::normalize_degrees(lower_outside) / 180, augs::normalize_degrees(upper_outside) / 180);
					s.v2.set(augs::normalize_degrees(lower_inside) / 180, augs::normalize_degrees(upper_inside) / 180);

					target.push_special_vertex_triangle(s, s, s);
					target.push_special_vertex_triangle(s, s, s);
				};

				push_angles(starting_health_angle, starting_health_angle + 90, starting_health_angle, ending_health_angle);

				struct circle_info {
					float angle;
					std::wstring text;
					rgba color;
				};

				std::vector<circle_info> textual_infos;

				if (v == watched_character) {
					const auto examine_item_slot = [&textual_infos, &push_angles, &circle_hud, &state](
						const const_inventory_slot_handle id,
						const float lower_outside,
						const float max_angular_length,
						const bool ccw
						) {
						if (id.alive() && id.has_items()) {
							const auto item = id.get_items_inside()[0];

							const auto ammo_info = get_ammunition_information(item);

							if (ammo_info.total_ammunition_space_available > 0) {
								const auto ammo_ratio = 1 - (ammo_info.total_actual_free_space / ammo_info.total_ammunition_space_available);

								auto redviolet = violet;
								redviolet.r = 200;
								circle_hud.color = augs::interp(white, redviolet, (1 - ammo_ratio)* (1 - ammo_ratio));
								circle_hud.color.a = 200;
								circle_hud.draw(state);

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

								new_info.text = to_wstring(ammo_info.total_charges);
								new_info.color = circle_hud.color;

								textual_infos.push_back(new_info);
							}
						}
					};

					examine_item_slot(v[slot_function::SECONDARY_HAND], starting_health_angle + 90.f + 22.5f, 45.f, false);
					examine_item_slot(v[slot_function::PRIMARY_HAND], starting_health_angle - 22.5f - 45.f, 45.f, true);
				}

				const int radius = (*assets::game_image_id::HUD_CIRCULAR_BAR_MEDIUM).get_size().x / 2;
				const auto empty_health_amount = static_cast<int>((1 - sentience->health.get_ratio()) * 90);

				textual_infos.push_back({
					starting_health_angle + 90 - empty_health_amount / 2,
					to_wstring(int(sentience->health.value) == 0 ? 1 : int(sentience->health.value)),
					health_col
				});

				textual_infos.push_back({
					starting_health_angle,
					get_bbcoded_entity_name(v),
					health_col
				});

				for (const auto& in : textual_infos) {
					if (in.text.empty()) {
						continue;
					}

					augs::gui::text_drawer health_points;
					health_points.set_text(augs::gui::text::format(in.text, augs::gui::text::style(assets::font_id::GUI_FONT, in.color)));

					//const auto circle_displacement_length = health_points.get_bbox().bigger_side() + radius;
					const vec2i screen_space_circle_center = r.camera[transform.pos];

					health_points.pos = screen_space_circle_center + position_rectangle_around_a_circle(radius + 6.f, health_points.get_bbox(), in.angle) - health_points.get_bbox() / 2;
					//health_points.pos = screen_space_circle_center + vec2().set_from_degrees(in.angle).set_length(circle_displacement_length);

					health_points.draw_stroke(circular_bars_information);
					health_points.draw(circular_bars_information);
				}
			}
		}

		return circular_bars_information;
	}
}