#pragma once
#include "augs/entity_system/entity_id.h"
#include "math/vec2.h"

#include "game_framework/assets/texture.h"
#include "game_framework/components/render_component.h"
#include "graphics/pixel.h"

namespace augs {
	class world;
}

namespace components {
	struct item;
	struct physics_definition;
}

namespace ingredients {
	void camera(augs::entity_id, int w, int h);

	components::item& make_item(augs::entity_id);
	
	void sprite(augs::entity_id, vec2 pos, assets::texture_id = assets::texture_id::BLANK, augs::rgba col = augs::rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	void sprite_scalled(augs::entity_id, vec2 pos, vec2i size, assets::texture_id = assets::texture_id::BLANK, augs::rgba col = augs::rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	
	components::physics_definition& crate_physics(augs::entity_id);
	components::physics_definition& static_crate_physics(augs::entity_id);

	void wsad_character_physics(augs::entity_id);

	void wsad_character_legs(augs::entity_id legs, augs::entity_id player);
	void wsad_character_crosshair(augs::entity_id);
	void wsad_character(augs::entity_id, augs::entity_id crosshair_entity);
	void inject_window_input_to_character(augs::entity_id target_character, augs::entity_id camera_entity);

	void make_always_visible(augs::entity_id);
	void cancel_always_visible(augs::entity_id);

	void character_inventory(augs::entity_id);
	void backpack(augs::entity_id);

	void assault_rifle(augs::entity_id);
}

namespace prefabs {
	augs::entity_id create_car(augs::world&, components::transform);
	augs::entity_id create_motorcycle(augs::world&, components::transform);

	augs::entity_id create_sample_magazine(augs::world&, vec2 pos);
	augs::entity_id create_sample_rifle(augs::world&, vec2 pos);
	
	augs::entity_id create_pink_charge(augs::world&, vec2 pos);

	augs::entity_id create_sample_backpack(augs::world&, vec2 pos);
}