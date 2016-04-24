#include "immediate_hud.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/systems/render_system.h"
#include "game/components/sprite_component.h"
#include "game/components/camera_component.h"
#include "game/components/sentience_component.h"

#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_id.h"

#include "graphics/renderer.h"
#include "graphics/vertex.h"

void immediate_hud::draw_circular_bars(messages::camera_render_request_message r) {
	auto& render = r.camera->get_owner_world().get_system<render_system>();
	const auto& visible_entities = render.get_all_visible_entities();
	auto& target = *r.state.output;

	auto camera = r.camera;
	auto watched_character = camera->get<components::camera>().entity_to_chase;

	for (auto v : render.get_all_visible_entities()) {
		auto* sentience = v->find<components::sentience>();

		if (sentience) {
			shared::state_for_drawing_renderable state;
			state.setup_camera_state(r.state);
			state.renderable_transform = v->get<components::transform>();
			state.renderable_transform.rotation = 0;

			components::sprite circle_hud;
			circle_hud.set(assets::HUD_CIRCULAR_BAR_MEDIUM, cyan);
			auto hsv_c = cyan.get_hsv();
			//auto hsv_r = rgba(225, 50, 56, 255).get_hsv();
			auto hsv_r = red.get_hsv();
			auto hr = sentience->health_ratio();

			hsv_c.h = augs::interp(hsv_c.h, hsv_r.h, (1 - hr)*(1 - hr)/*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*/);
			hsv_c.s = augs::interp(hsv_c.s, hsv_r.s, (1 - hr)*(1 - hr)/*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*/);
			hsv_c.v = augs::interp(hsv_c.v, hsv_r.v, (1 - hr)*(1 - hr)/*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*(1 - hr)*/);
			circle_hud.color.set_hsv(hsv_c);
			circle_hud.color.a = 220;
			circle_hud.color.b = std::min(circle_hud.color.b + 120, 255);
			circle_hud.color.g = std::min(int(circle_hud.color.g), circle_hud.color.b+40);
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

			if (v == watched_character) {
				auto primary = v[slot_function::PRIMARY_HAND];
				auto secondary = v[slot_function::SECONDARY_HAND];

				if (secondary.alive() && secondary.has_items()) {
					auto maybe_magazine_slot = secondary->items_inside[0][slot_function::GUN_DETACHABLE_MAGAZINE];

					if (maybe_magazine_slot.alive()) {
						float ammo_ratio = 0.f;

						if (maybe_magazine_slot.has_items()) {
							auto ammo_depo = maybe_magazine_slot->items_inside[0][slot_function::ITEM_DEPOSIT];

							ammo_ratio = 1 - (ammo_depo->calculate_free_space_with_children() / float(ammo_depo->space_available));
						}

						circle_hud.color = augs::white;
						circle_hud.color.a = 150;
						circle_hud.draw(state);

						push_angles(starting_health_angle + 90, starting_health_angle + 90 + ammo_ratio * 90.f);
					}
				}
			}


			//state.renderable_transform.rotation = 90;
			//border.color = orange;
			//border.color.a = 160;
			//border.draw(state);
		}
	}
}

void immediate_hud::draw_circular_bars_information(messages::camera_render_request_message) {

}
