#include "immediate_hud.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/systems/render_system.h"
#include "game/systems/gui_system.h"
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
#include "stream.h"
#include "augs/gui/text_drawer.h"

void immediate_hud::draw_circular_bars(messages::camera_render_request_message r) {
	auto& render = r.camera->get_owner_world().get_system<render_system>();
	const auto& visible_entities = render.get_all_visible_entities();
	auto& target = *r.state.output;

	auto camera = r.camera;
	auto watched_character = camera->get<components::camera>().entity_to_chase;

	int timestamp_ms = render.frame_timestamp_seconds() * 1000;

	circular_bars_information.clear();
	pure_color_highlights.clear();

	for (auto v : render.get_all_visible_entities()) {
		auto* sentience = v->find<components::sentience>();

		if (sentience) {
			auto hr = sentience->health_ratio();
			auto one_less_hr = 1 - hr;

			int pulse_duration = 1250 - 1000 * (1 - hr);
			float time_pulse_ratio = (timestamp_ms % pulse_duration) / float(pulse_duration);

			hr *= 1.f - (0.2f * time_pulse_ratio);

			auto* render = v->find<components::render>();
			
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

			auto& transform = v->get<components::transform>();
			shared::state_for_drawing_renderable state;
			state.setup_camera_state(r.state);
			state.renderable_transform = transform;
			state.renderable_transform.rotation = 0;

			components::sprite circle_hud;
			circle_hud.set(assets::HUD_CIRCULAR_BAR_MEDIUM, health_col);
			circle_hud.draw(state);

			augs::special special_vertex_data;
			
			auto watched_character_transform = watched_character->get<components::transform>();
			float starting_health_angle = 0.f;
			float ending_health_angle = 0.f;

			if (v == watched_character) {
				starting_health_angle = watched_character_transform.rotation + 135;
				ending_health_angle = starting_health_angle + sentience->health_ratio() * 90.f;
			}
			else {
				starting_health_angle = (v->get<components::transform>().pos - watched_character_transform.pos).degrees() - 45;
				ending_health_angle = starting_health_angle + sentience->health_ratio() * 90.f;
			}

			auto push_angles = [&](float first, float second) {
				float first_angle = normalize_degrees(first);
				float second_angle = normalize_degrees(second);

				special_vertex_data.v1.set(first_angle / 180, second_angle / 180);

				target.push_special_vertex_triangle(special_vertex_data, special_vertex_data, special_vertex_data);
				target.push_special_vertex_triangle(special_vertex_data, special_vertex_data, special_vertex_data);
			};
			
			push_angles(starting_health_angle, ending_health_angle);

			struct circle_info {
				float angle;
				std::wstring text;
				rgba color;
			} infos[4];

			if (v == watched_character) {
				auto primary = v[slot_function::PRIMARY_HAND];
				auto secondary = v[slot_function::SECONDARY_HAND];

				if (secondary.alive() && secondary.has_items()) {
					auto item = secondary->items_inside[0];

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
						total_actual_free_space += ammo_depo->calculate_free_space_with_children();
					}

					if (chamber_slot.alive()) {
						charges += count_charges_inside(chamber_slot);

						total_space_available += chamber_slot->space_available;
						total_actual_free_space += chamber_slot->calculate_free_space_with_children();
					}

					ammo_ratio = 1 - (total_actual_free_space / total_space_available);

					auto redviolet = augs::violet;
					redviolet.r = 200;
					circle_hud.color = augs::interp(augs::white, redviolet, (1 - ammo_ratio)* (1 - ammo_ratio));
					circle_hud.color.a = 200;
					circle_hud.draw(state);

					push_angles(starting_health_angle + 90, starting_health_angle + 90 + ammo_ratio * 90.f);
					infos[3].angle = starting_health_angle + 90 + 90;
					infos[3].text = augs::to_wstring(charges);
					infos[3].color = circle_hud.color;
				}
			}

			int radius = (*assets::HUD_CIRCULAR_BAR_MEDIUM).get_size().x / 2;

			infos[0] = { starting_health_angle + 90, augs::to_wstring(sentience->health), health_col };
			infos[1] = { starting_health_angle, description_of_entity(v).name, health_col };

			for (auto& in : infos) {
				if (in.text.empty()) continue;

				augs::gui::text_drawer health_points;
				health_points.set_text(augs::gui::text::format(in.text, augs::gui::text::style(assets::GUI_FONT, in.color)));

				auto circle_displacement_length = health_points.get_bbox().bigger_side() + radius;
				vec2i screen_space_circle_center = r.get_screen_space(transform.pos);

				health_points.pos = screen_space_circle_center + vec2().set_from_degrees(in.angle).set_length(circle_displacement_length);

				health_points.draw_stroke(circular_bars_information);
				health_points.draw(circular_bars_information);
			}
		}
	}
}

void immediate_hud::draw_circular_bars_information(messages::camera_render_request_message r) {
	r.state.output->triangles.insert(r.state.output->triangles.begin(), circular_bars_information.begin(), circular_bars_information.end());
}

void immediate_hud::draw_pure_color_highlights(messages::camera_render_request_message r) {
	r.state.output->triangles.insert(r.state.output->triangles.begin(), pure_color_highlights.begin(), pure_color_highlights.end());
}

void immediate_hud::acquire_game_events(augs::world& w) {
	auto& healths = w.get_message_queue<messages::health_event>();

	for (auto& h : healths) {
		game_event_visualization new_event;
		new_event.time_of_occurence = w.get_current_timestamp();
		new_event.type = game_event_visualization::VERTICALLY_FLYING_NUMBER;
		new_event.value = h.effective_amount;
		new_event.maximum_duration_seconds = 0.7;

		augs::rgba col;

		if (h.target == messages::health_event::HEALTH) {
			if (h.effective_amount > 0) {
				col = red;
			}
			else col = green;
		}
		else
			continue;

		new_event.text.set_text(augs::gui::text::format(augs::to_wstring(std::abs(new_event.value)), augs::gui::text::style(assets::GUI_FONT, col)));
		new_event.transform.pos = h.point_of_impact;

		recent_game_events.push_back(new_event);
	}
}

void immediate_hud::visualize_recent_game_events(messages::camera_render_request_message msg) {
	auto& world = msg.camera->get_owner_world();
	auto& target = renderer::get_current();

	auto current_time = world.get_current_timestamp() + world.parent_overworld.fixed_delta_milliseconds()/1000 * world.parent_overworld.view_interpolation_ratio();
	
	for (auto& r : recent_game_events) { 
		auto passed = current_time - r.time_of_occurence;
		auto ratio =  passed / r.maximum_duration_seconds;
		
		if (r.type == game_event_visualization::VERTICALLY_FLYING_NUMBER) {

			r.text.pos = msg.get_screen_space((r.transform.pos - vec2(0, sqrt(passed) * 150.f)));
			
			r.text.draw_stroke(msg.state.output->triangles);
			r.text.draw(msg.state.output->triangles);
		}
	}

	recent_game_events.erase(std::remove_if(recent_game_events.begin(), recent_game_events.end(), [current_time](const game_event_visualization& v) {
		return (current_time - v.time_of_occurence) > v.maximum_duration_seconds;
	}), recent_game_events.end());
}