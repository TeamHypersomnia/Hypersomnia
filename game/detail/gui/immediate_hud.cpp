#include "immediate_hud.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/systems/render_system.h"
#include "game/components/sprite_component.h"

void immediate_hud::draw_circular_bars_and_nicknames(messages::camera_render_request_message r) {
	auto& render = r.camera->get_owner_world().get_system<render_system>();
	const auto& visible_entities = render.get_all_visible_entities();

	for (auto v : render.get_all_visible_entities()) {
		shared::state_for_drawing_renderable state;
		state.setup_camera_state(r.state);
		state.renderable_transform = v->get<components::transform>();
		state.renderable_transform.rotation = 0;

		components::sprite border;
		border.set(assets::HUD_CIRCULAR_BAR_MEDIUM, cyan);
		border.draw(state);
	}
}