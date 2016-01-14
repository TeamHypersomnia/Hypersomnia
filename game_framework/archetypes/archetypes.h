#pragma once
#include "utilities/entity_system/entity_id.h"
#include "math/vec2.h"

#include "game_framework/assets/texture.h"
#include "game_framework/components/render_component.h"
#include "graphics/pixel.h"

namespace augs {
	class world;
}

namespace archetypes {
	void camera(augs::entity_id, int w, int h);
	void sprite(augs::entity_id, vec2 pos, assets::texture_id = assets::texture_id::BLANK, augs::pixel_32 col = augs::pixel_32(255, 255, 255, 255), components::render::render_layer = components::render::render_layer::GROUND);
	void sprite_scalled(augs::entity_id, vec2 pos, vec2i size, assets::texture_id = assets::texture_id::BLANK, augs::pixel_32 col = augs::pixel_32(255, 255, 255, 255), components::render::render_layer = components::render::render_layer::GROUND);
	
	void crate_physics(augs::entity_id);
	void static_crate_physics(augs::entity_id);

	void wsad_player_physics(augs::entity_id);

	void wsad_player_legs(augs::entity_id legs, augs::entity_id player);
	void wsad_player_crosshair(augs::entity_id);
	void wsad_player(augs::entity_id, augs::entity_id crosshair_entity, augs::entity_id camera_entity);
}

namespace prefabs {
	augs::entity_id create_car(augs::world&, vec2 pos);
}