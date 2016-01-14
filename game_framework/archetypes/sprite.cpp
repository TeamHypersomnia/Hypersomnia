#include "archetypes.h"
#include "renderable_includes.h"

namespace archetypes {
	void sprite(augs::entity_id e, vec2 pos, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		components::sprite sprite;
		components::render render;
		components::transform transform;

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);

		e->add(render);
		e->add(sprite);
		e->add(transform);
	}

	void sprite_scalled(augs::entity_id e, vec2 pos, vec2i size, assets::texture_id id, augs::pixel_32 col, components::render::render_layer layer) {
		sprite(e, pos, id, col, layer);
		e->get<components::sprite>().size = size;
	}
}