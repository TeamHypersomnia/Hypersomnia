#include "immediate_hud.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"
#include "game/components/sprite_component.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/physics_component.h"
#include "game/components/name_component.h"
#include "game/components/container_component.h"

#include "game/messages/health_event.h"

#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/entity_description.h"

#include "augs/graphics/renderer.h"
#include "augs/graphics/vertex.h"
#include "augs/gui/text_drawer.h"

#include "augs/misc/stepped_timing.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/string_templates.h"

vec2 position_caption_around_a_circle(const float radius, const vec2 r, const float alpha) {
	const vec2 top_bounds[2] =  { vec2(-r.x / 2, -radius - r.y / 2), vec2(r.x / 2, -radius - r.y / 2) };
	const vec2 left_bounds[2] = { vec2(-radius - r.x / 2, r.y / 2), vec2(-radius - r.x / 2, -r.y / 2) };
	const vec2 bottom_bounds[2] = { top_bounds[1] * vec2(1, -1), top_bounds[0] * vec2(1, -1) };
	const vec2 right_bounds[2] = { left_bounds[1] * vec2(-1, 1), left_bounds[0] * vec2(-1, 1) };

	const vec2 all_bounds[4][2] = {
		{ left_bounds[0], left_bounds[1] }, 
		{ top_bounds[0], top_bounds[1] }, 
		{ right_bounds[0], right_bounds[1] }, 
		{ bottom_bounds[0], bottom_bounds[1] }  
	};

	const vec2 angle_norm = vec2().set_from_degrees(alpha);
	const vec2 angle = angle_norm * radius;

	static const vec2 quadrant_multipliers[4] = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1), };
	static const vec2 quadrants_on_circle[4] = { vec2(-1, 0), vec2(0, -1), vec2(1, 0), vec2(0, 1), };

	for (int i = 0; i < 4; ++i) {
		const vec2 a = vec2(all_bounds[i][0]).normalize(),
			b = vec2(all_bounds[i][1]).normalize(),
			c = vec2(all_bounds[(i+1)%4][0]).normalize(),
			v = angle_norm;

		float bound_angular_distance = a.cross(b);
		float target_angular_distance = a.cross(v);

		if (target_angular_distance >= 0 && b.cross(v) <= 0) {
			return augs::interp(all_bounds[i][0], all_bounds[i][1], target_angular_distance / bound_angular_distance);
		}
		else {
			bound_angular_distance = b.cross(c);
			target_angular_distance = b.cross(v);

			if (target_angular_distance >= 0.0 && c.cross(v) <= 0.0) {
				return vec2(quadrants_on_circle[i]).rotate((target_angular_distance / bound_angular_distance) * 90, vec2(0, 0)) * radius + quadrant_multipliers[i] * r / 2;
			}
		}
	}
	
	//ensure(false);
	return vec2(0, 0);
}

augs::vertex_triangle_buffer immediate_hud::draw_circular_bars_and_get_textual_info(viewing_step& r) const {
	const auto& dynamic_tree = r.cosm.systems_temporary.get<dynamic_tree_system>();
	const auto& visible_entities = r.visible_entities;
	auto& target = r.renderer;
	const auto& cosmos = r.cosm;
	const auto& interp = r.session.systems_audiovisual.get<interpolation_system>();

	const auto& watched_character = cosmos[r.viewed_character];

	const auto timestamp_ms = static_cast<unsigned>(r.get_interpolated_total_time_passed_in_seconds() * 1000);

	augs::vertex_triangle_buffer circular_bars_information;

	for (const auto v : visible_entities) {
		const auto* const sentience = v.find<components::sentience>();

		if (sentience) {
			const auto hr = sentience->health.ratio();
			const auto one_less_hr = 1 - hr;

			const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));
			const float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

			const auto health_col = sentience->calculate_health_color(time_pulse_ratio);

			auto& transform = v.viewing_transform(interp);
			
			components::sprite::drawing_input state(r.renderer.triangles);
			state.camera = r.camera;
			state.renderable_transform = transform;
			state.renderable_transform.rotation = 0;

			components::sprite circle_hud;
			circle_hud.set(assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM, health_col);
			circle_hud.draw(state);
			
			const auto watched_character_transform = watched_character.viewing_transform(r.session.systems_audiovisual.get<interpolation_system>());
			float starting_health_angle = 0.f;
			float ending_health_angle = 0.f;

			if (v == watched_character) {
				starting_health_angle = watched_character_transform.rotation + 135;
				ending_health_angle = starting_health_angle + sentience->health.ratio() * 90.f;
			}
			else {
				starting_health_angle = (v.viewing_transform(interp).pos - watched_character_transform.pos).degrees() - 45;
				ending_health_angle = starting_health_angle + sentience->health.ratio() * 90.f;
			}

			auto push_angles = [&target](const float lower_outside, const float upper_outside, const float lower_inside, const float upper_inside) {
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
				auto examine_item_slot = [&textual_infos, &push_angles, &circle_hud, &state](const_inventory_slot_handle id, float lower_outside, float max_angular_length, bool ccw) {
					if (id.alive() && id.has_items()) {
						const auto& item = id.get_items_inside()[0];

						const auto& maybe_magazine_slot = item[slot_function::GUN_DETACHABLE_MAGAZINE];
						const auto& chamber_slot = item[slot_function::GUN_CHAMBER];

						float ammo_ratio = 0.f;
						int charges = 0;
						float total_space_available = 0.f;
						float total_actual_free_space = 0.f;

						if (maybe_magazine_slot.alive() && maybe_magazine_slot.has_items()) {
							auto mag = maybe_magazine_slot.get_items_inside()[0];
							auto ammo_depo = mag[slot_function::ITEM_DEPOSIT];
							charges += count_charges_in_deposit(mag);

							total_space_available += ammo_depo->space_available;
							total_actual_free_space += ammo_depo.calculate_free_space_with_children();
						}

						if (chamber_slot.alive()) {
							charges += count_charges_inside(chamber_slot);

							total_space_available += chamber_slot->space_available;
							total_actual_free_space += chamber_slot.calculate_free_space_with_children();
						}

						if (total_space_available > 0) {
							ammo_ratio = 1 - (total_actual_free_space / total_space_available);

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
								new_info.angle = upper_outside - empty_amount/2;
							}
							else {
								push_angles(lower_outside, upper_outside, upper_outside - ammo_ratio * max_angular_length, upper_outside);
								new_info.angle = lower_outside + empty_amount / 2;
							}

							new_info.text = to_wstring(charges);
							new_info.color = circle_hud.color;

							textual_infos.push_back(new_info);
						}
					}
				};

				examine_item_slot(v[slot_function::SECONDARY_HAND], starting_health_angle + 90.f + 22.5f, 45.f, false);
				examine_item_slot(v[slot_function::PRIMARY_HAND], starting_health_angle - 22.5f - 45.f, 45.f, true);
			}

			const int radius = (*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_size().x / 2;
			const auto empty_health_amount = static_cast<int>((1 - sentience->health.ratio()) * 90);

			textual_infos.push_back({ starting_health_angle + 90 - empty_health_amount/2, to_wstring(int(sentience->health.value) == 0 ? 1 : int(sentience->health.value)), health_col });
			textual_infos.push_back({ starting_health_angle, description_of_entity(v).name, health_col });

			for (auto& in : textual_infos) {
				if (in.text.empty()) continue;

				augs::gui::text_drawer health_points;
				health_points.set_text(augs::gui::text::format(in.text, augs::gui::text::style(assets::font_id::GUI_FONT, in.color)));

				//const auto circle_displacement_length = health_points.get_bbox().bigger_side() + radius;
				const vec2i screen_space_circle_center = r.get_screen_space(transform.pos);

				health_points.pos = screen_space_circle_center + position_caption_around_a_circle(radius+6.f, health_points.get_bbox(), in.angle) - health_points.get_bbox()/2;
				//health_points.pos = screen_space_circle_center + vec2().set_from_degrees(in.angle).set_length(circle_displacement_length);

				health_points.draw_stroke(circular_bars_information);
				health_points.draw(circular_bars_information);
			}
		}
	}

	return circular_bars_information;
}

void immediate_hud::acquire_game_events(const const_logic_step& step) {
	const auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	const auto current_time = cosmos.get_total_time_passed_in_seconds();

	for (const auto& h : healths) {
		vertically_flying_number vn;
		vn.time_of_occurence = current_time;
		vn.value = h.effective_amount;
		vn.maximum_duration_seconds = 0.7;

		rgba col;

		if (h.target == messages::health_event::HEALTH) {
			if (h.effective_amount > 0) {
				col = red;
			}
			else {
				col = green;
			}
		}
		else {
			continue;
		}

		vn.text.set_text(augs::gui::text::format(to_wstring(std::abs(int(vn.value) == 0 ? 1 : int(vn.value))), augs::gui::text::style(assets::font_id::GUI_FONT, col)));
		vn.transform.pos = h.point_of_impact;

		recent_vertically_flying_numbers.push_back(vn);

		if (cosmos[h.spawned_remnants].alive()) {
			vn.text.set_text(augs::gui::text::format(L"Death", augs::gui::text::style(assets::font_id::GUI_FONT, col)));
			vn.transform.pos = cosmos[h.spawned_remnants].logic_transform().pos;
			recent_vertically_flying_numbers.push_back(vn);
		}

		pure_color_highlight new_highlight;
		new_highlight.time_of_occurence = cosmos.get_total_time_passed_in_seconds();
		
		new_highlight.target = h.subject;
		new_highlight.starting_alpha_ratio = std::min(1.f, h.ratio_effective_to_maximum * 5);
		
		if (cosmos[h.spawned_remnants].alive()) {
			new_highlight.target = h.spawned_remnants;
			new_highlight.starting_alpha_ratio = 0.7f;
		}

		new_highlight.maximum_duration_seconds = 0.3;
		new_highlight.color = col;

		erase_remove(recent_pure_color_highlights, [&new_highlight, &cosmos](const pure_color_highlight& existing_highlight) { 
			return existing_highlight.target == new_highlight.target;
		});

		recent_pure_color_highlights.push_back(new_highlight);
	}

	auto timeout_lambda = [current_time](const game_event_visualization& v) {
		return (current_time - v.time_of_occurence) > v.maximum_duration_seconds;
	};

	erase_remove(recent_vertically_flying_numbers, timeout_lambda);
	erase_remove(recent_pure_color_highlights, timeout_lambda);
}

void immediate_hud::draw_vertically_flying_numbers(viewing_step& msg) const {
	const auto current_time = msg.get_interpolated_total_time_passed_in_seconds();
	auto& triangles = msg.renderer.triangles;

	for (const auto& r : recent_vertically_flying_numbers) { 
		auto passed = current_time - r.time_of_occurence;
		auto ratio =  passed / r.maximum_duration_seconds;

		auto text = r.text;

		text.pos = msg.get_screen_space((r.transform.pos - vec2(0, static_cast<float>(sqrt(passed)) * 120.f)));
		
		text.draw_stroke(triangles);
		text.draw(triangles);
	}
}

void immediate_hud::draw_pure_color_highlights(viewing_step& msg) const {
	const auto& cosmos = msg.cosm;
	const auto current_time = static_cast<float>(msg.get_interpolated_total_time_passed_in_seconds());
	auto& triangles = msg.renderer.triangles;
	const auto& interp = msg.session.systems_audiovisual.get<interpolation_system>();

	for (const auto& r : recent_pure_color_highlights) {
		const auto& subject = cosmos[r.target];

		if (subject.dead() || !subject.has<components::sprite>())
			continue;

		auto sprite = subject.get<components::sprite>();
		auto& col = sprite.color;
		auto prevcol = col;
		col = r.color;

		auto passed = current_time - r.time_of_occurence;
		auto ratio = passed / r.maximum_duration_seconds;

		col.a = static_cast<rgba_channel>(255 * (1-ratio) * r.starting_alpha_ratio);
		render_system().draw_renderable(triangles, current_time, sprite, subject.viewing_transform(interp, true), subject.get<components::render>(), msg.camera, renderable_drawing_type::NORMAL);
		col = prevcol;
	}

	//msg.state.output->triangles.insert(msg.state.output->triangles.begin(), pure_color_highlights.begin(), pure_color_highlights.end());
}