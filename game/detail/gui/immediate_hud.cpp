#include "immediate_hud.h"
#include "game/entity_id.h"
#include "game/cosmos.h"

#include "game/systems/render_system.h"
#include "game/stateful_systems/gui_system.h"
#include "game/components/sprite_component.h"
#include "game/components/camera_component.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/physics_component.h"
#include "game/components/name_component.h"

#include "game/messages/health_event.h"

#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/entity_description.h"

#include "graphics/renderer.h"
#include "graphics/vertex.h"
#include "templates.h"
#include "templates.h"
#include "augs/gui/text_drawer.h"

vec2 position_caption_around_a_circle(float radius, vec2 r, float alpha) {
	vec2 top_bounds[2] =  { vec2(-r.x / 2, -radius - r.y / 2), vec2(r.x / 2, -radius - r.y / 2) };
	vec2 left_bounds[2] = { vec2(-radius - r.x / 2, r.y / 2), vec2(-radius - r.x / 2, -r.y / 2) };
	vec2 bottom_bounds[2] = { top_bounds[1] * vec2(1, -1), top_bounds[0] * vec2(1, -1) };
	vec2 right_bounds[2] = { left_bounds[1] * vec2(-1, 1), left_bounds[0] * vec2(-1, 1) };

	vec2 all_bounds[4][2] = { 
		{ left_bounds[0], left_bounds[1] }, 
		{ top_bounds[0], top_bounds[1] }, 
		{ right_bounds[0], right_bounds[1] }, 
		{ bottom_bounds[0], bottom_bounds[1] }  
	};

	vec2 angle_norm = vec2().set_from_degrees(alpha);
	vec2 angle = angle_norm * radius;

	static vec2 quadrant_multipliers[4] = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1), };
	static vec2 quadrants_on_circle[4] = { vec2(-1, 0), vec2(0, -1), vec2(1, 0), vec2(0, 1), };

	for (int i = 0; i < 4; ++i) {
		vec2 a = vec2(all_bounds[i][0]).normalize(), 
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

void immediate_hud::draw_circular_bars(messages::camera_render_request_message r) {
	auto& render = r.camera->get_owner_world().systems.get<dynamic_tree_system>();
	const auto& visible_entities = render.get_all_visible_entities();
	auto& target = *r.state.output;

	auto camera = r.camera;
	auto watched_character = camera.get<components::camera>().entity_to_chase;

	int timestamp_ms = render.frame_timestamp_seconds() * 1000;

	circular_bars_information.clear();
	pure_color_highlights.clear();

	for (auto v : render.get_all_visible_entities()) {
		auto* sentience = v.find<components::sentience>();

		if (sentience) {
			auto hr = sentience->health.ratio();
			auto one_less_hr = 1 - hr;

			int pulse_duration = 1250 - 1000 * (1 - hr);
			float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

			hr *= 1.f - (0.2f * time_pulse_ratio);

			auto* render = v.find<components::render>();
			
			if (render) {
				//render->partial_overlay_color = red;
				//render->partial_overlay_height_ratio = 1 - hr;
				if (hr < 1.f) {
					render->draw_border = true;
					render->border_color = rgba(255, 0, 0, one_less_hr * one_less_hr * one_less_hr * one_less_hr * 255 * time_pulse_ratio);
				}
				else
					render->draw_border = false;
			}

			auto health_col = sentience->calculate_health_color(time_pulse_ratio);

			auto& transform = v.get<components::transform>();
			shared::state_for_drawing_renderable state;
			state.setup_camera_state(r.state);
			state.renderable_transform = transform;
			state.renderable_transform.rotation = 0;

			components::sprite circle_hud;
			circle_hud.set(assets::HUD_CIRCULAR_BAR_MEDIUM, health_col);
			circle_hud.draw(state);

			
			auto watched_character_transform = watched_character.get<components::transform>();
			float starting_health_angle = 0.f;
			float ending_health_angle = 0.f;

			if (v == watched_character) {
				starting_health_angle = watched_character_transform.rotation + 135;
				ending_health_angle = starting_health_angle + sentience->health.ratio() * 90.f;
			}
			else {
				starting_health_angle = (v.get<components::transform>().pos - watched_character_transform.pos).degrees() - 45;
				ending_health_angle = starting_health_angle + sentience->health.ratio() * 90.f;
			}

			auto push_angles = [&target](float lower_outside, float upper_outside, float lower_inside, float upper_inside) {
				augs::special s;

				s.v1.set(normalize_degrees(lower_outside) / 180, normalize_degrees(upper_outside) / 180);
				s.v2.set(normalize_degrees(lower_inside) / 180, normalize_degrees(upper_inside) / 180);

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
				auto examine_item_slot = [&textual_infos, &push_angles, &circle_hud, &state](inventory_slot_id id, float lower_outside, float max_angular_length, bool ccw) {
					if (id.alive() && id.has_items()) {
						auto item = id->items_inside[0];

						auto maybe_magazine_slot = item[slot_function::GUN_DETACHABLE_MAGAZINE];
						auto chamber_slot = item[slot_function::GUN_CHAMBER];

						float ammo_ratio = 0.f;
						int charges = 0;
						float total_space_available = 0.f;
						float total_actual_free_space = 0.f;

						if (maybe_magazine_slot.alive() && maybe_magazine_slot.has_items()) {
							auto mag = maybe_magazine_slot->items_inside[0];
							auto ammo_depo = mag[slot_function::ITEM_DEPOSIT];
							charges += count_charges_in_deposit(mag);

							total_space_available += ammo_depo->space_available;
							total_actual_free_space += ammo_depo.calculate_free_space_with_children();
						}

						if (chamber_slot.alive()) {
							charges += count_charges_inside(chamber_slot);

							total_space_available += chamber_slot->space_available;
							total_actual_free_space += chamber_slot->calculate_free_space_with_children();
						}

						if (total_space_available > 0) {
							ammo_ratio = 1 - (total_actual_free_space / total_space_available);

							auto redviolet = augs::violet;
							redviolet.r = 200;
							circle_hud.color = augs::interp(augs::white, redviolet, (1 - ammo_ratio)* (1 - ammo_ratio));
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

				examine_item_slot(v[slot_function::SECONDARY_HAND], starting_health_angle + 90 + 22.5, 90, false);
				examine_item_slot(v[slot_function::PRIMARY_HAND], starting_health_angle - 90 - 22.5, 90, true);
			}

			int radius = (*assets::HUD_CIRCULAR_BAR_MEDIUM).get_size().x / 2;

			int empty_health_amount = (1 - sentience->health.ratio()) * 90;

			textual_infos.push_back({ starting_health_angle + 90 - empty_health_amount/2, to_wstring(int(sentience->health.value)), health_col });
			textual_infos.push_back({ starting_health_angle, description_of_entity(v).name, health_col });

			for (auto& in : textual_infos) {
				if (in.text.empty()) continue;

				augs::gui::text_drawer health_points;
				health_points.set_text(augs::gui::text::format(in.text, augs::gui::text::style(assets::GUI_FONT, in.color)));

				auto circle_displacement_length = health_points.get_bbox().bigger_side() + radius;
				vec2i screen_space_circle_center = r.get_screen_space(transform.pos);


				health_points.pos = screen_space_circle_center + position_caption_around_a_circle(radius+6, health_points.get_bbox(), in.angle) - health_points.get_bbox()/2;
				//health_points.pos = screen_space_circle_center + vec2().set_from_degrees(in.angle).set_length(circle_displacement_length);

				health_points.draw_stroke(circular_bars_information);
				health_points.draw(circular_bars_information);
			}
		}
	}
}

void immediate_hud::draw_circular_bars_information(messages::camera_render_request_message r) {
	r.state.output->triangles.insert(r.state.output->triangles.begin(), circular_bars_information.begin(), circular_bars_information.end());
}

void immediate_hud::acquire_game_events(cosmos& cosmos, step_state& step) {
	auto& healths = step.messages.get_queue<messages::health_event>();
	auto& delta = cosmos.delta;

	for (auto& h : healths) {
		vertically_flying_number vn;
		vn.time_of_occurence = delta.total_time_passed_in_seconds();
		vn.value = h.effective_amount;
		vn.maximum_duration_seconds = 0.7;

		augs::rgba col;

		if (h.target == messages::health_event::HEALTH) {
			if (h.effective_amount > 0) {
				col = red;
			}
			else col = green;
		}
		else
			continue;

		vn.text.set_text(augs::gui::text::format(to_wstring(std::abs(int(vn.value))), augs::gui::text::style(assets::GUI_FONT, col)));
		vn.transform.pos = h.point_of_impact;

		recent_vertically_flying_numbers.push_back(vn);

		pure_color_highlight ph;
		ph.time_of_occurence = w.get_current_timestamp();
		
		ph.target = h.subject;
		ph.starting_alpha_ratio = std::min(1.f, h.ratio_effective_to_maximum * 5);
		
		if (cosmos[h.spawned_remnants].alive()) {
			ph.target = h.spawned_remnants;
			ph.starting_alpha_ratio = 0.7;
		}

		ph.maximum_duration_seconds = 0.3;
		ph.color = col;

		erase_remove(recent_pure_color_highlights, [&ph](const pure_color_highlight& h) { return h.target == ph.target || h.target == ph.target[associated_entity_name::ASTRAL_BODY]; });
		recent_pure_color_highlights.push_back(ph);
	}
}

double immediate_hud::get_current_time(messages::camera_render_request_message msg) const {
	auto& world = msg.camera->get_owner_world();
	return world.get_current_timestamp() + world.parent_overworld.fixed_delta_milliseconds() / 1000 * world.parent_overworld.view_interpolation_ratio();
}

void immediate_hud::draw_vertically_flying_numbers(messages::camera_render_request_message msg) {
	auto& target = renderer::get_current();
	
	auto current_time = get_current_time(msg);

	for (auto& r : recent_vertically_flying_numbers) { 
		auto passed = current_time - r.time_of_occurence;
		auto ratio =  passed / r.maximum_duration_seconds;

		r.text.pos = msg.get_screen_space((r.transform.pos - vec2(0, sqrt(passed) * 150.f)));
		
		r.text.draw_stroke(msg.state.output->triangles);
		r.text.draw(msg.state.output->triangles);
	}

	auto timeout_lambda = [current_time](const game_event_visualization& v) {
		return (current_time - v.time_of_occurence) > v.maximum_duration_seconds;
	};

	erase_remove(recent_vertically_flying_numbers, timeout_lambda);
	erase_remove(recent_pure_color_highlights, timeout_lambda);
}

void immediate_hud::draw_pure_color_highlights(messages::camera_render_request_message msg) {
	auto current_time = get_current_time(msg);

	for (auto& r : recent_pure_color_highlights) {
		auto& col = r.target.get<components::sprite>().color;
		auto prevcol = col;
		col = r.color;

		auto passed = current_time - r.time_of_occurence;
		auto ratio = passed / r.maximum_duration_seconds;

		col.a = 255 * (1-ratio) * r.starting_alpha_ratio;
		render_system::standard_draw_entity(r.target, msg.state);
		col = prevcol;
	}

	//msg.state.output->triangles.insert(msg.state.output->triangles.begin(), pure_color_highlights.begin(), pure_color_highlights.end());
}