#include "archetypes.h"
#include "renderable_includes.h"

#include "../game/body_helper.h"
#include "../globals/filters.h"

namespace archetypes {
	void static_sprite(augs::entity_id e, vec2 pos, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& transform = *e += components::transform();

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);

		helpers::body_info body;
		body.body_type = b2_staticBody;

		helpers::physics_info info;
		info.from_renderable(e);
		info.sensor = true;

		info.filter = filters::renderable();
		info.density = 1;

		helpers::create_physics_component(body, e);
		helpers::add_fixtures(info, e);
	}

	void static_sprite_scalled(augs::entity_id e, vec2 pos, vec2i size, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		static_sprite(e, pos, id, col, layer);
		e->get<components::sprite>().size = size;
	}

	void sprite(augs::entity_id e, vec2 pos, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& transform = *e += components::transform();

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);
	}

	void sprite_scalled(augs::entity_id e, vec2 pos, vec2i size, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		sprite(e, pos, id, col, layer);
		e->get<components::sprite>().size = size;
	}
}