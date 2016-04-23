#include "immediate_hud.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/systems/render_system.h"
#include "game/components/sprite_component.h"

#include "graphics/renderer.h"
#include "graphics/vertex.h"

void immediate_hud::draw_circular_bars(messages::camera_render_request_message r) {
	auto& render = r.camera->get_owner_world().get_system<render_system>();
	const auto& visible_entities = render.get_all_visible_entities();
	auto& target = *r.state.output;

	for (auto v : render.get_all_visible_entities()) {
		auto* sentience = v->find<components::sentience>();

		if (sentience) {
			shared::state_for_drawing_renderable state;
			state.setup_camera_state(r.state);
			state.renderable_transform = v->get<components::transform>();
			state.renderable_transform.rotation = 0;

			components::sprite border;
			border.set(assets::HUD_CIRCULAR_BAR_MEDIUM, white);
			//border.color.a = 160;
			border.draw(state);
			
			augs::special special_vertex_data;
			special_vertex_data.v1;

			target.push_special_vertex_triangle(special_vertex_data, special_vertex_data, special_vertex_data);
			target.push_special_vertex_triangle(special_vertex_data, special_vertex_data, special_vertex_data);
			//state.renderable_transform.rotation = 90;
			//border.color = orange;
			//border.color.a = 160;
			//border.draw(state);
		}
	}
}

void immediate_hud::draw_circular_bars_information(messages::camera_render_request_message) {

}
