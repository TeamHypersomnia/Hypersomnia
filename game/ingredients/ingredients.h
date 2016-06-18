#pragma once
#include "game/entity_id.h"
#include "math/vec2.h"

#include "game/assets/texture_id.h"
#include "game/components/render_component.h"

#include "graphics/pixel.h"

class definition_interface;
class cosmos;

namespace components {
	struct item;
	struct physics_definition;
	struct sprite;
}

namespace ingredients {
	void camera(definition_interface, int w, int h);

	components::item& make_item(definition_interface);
	
	components::sprite& sprite(definition_interface, vec2 pos, assets::texture_id = assets::texture_id::BLANK, augs::rgba col = augs::rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	components::sprite& sprite_scalled(definition_interface, vec2 pos, vec2i size, assets::texture_id = assets::texture_id::BLANK, augs::rgba col = augs::rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	
	components::physics_definition& bullet_round_physics(definition_interface);
	components::physics_definition& see_through_dynamic_body(definition_interface);
	components::physics_definition& standard_dynamic_body(definition_interface);
	components::physics_definition& standard_static_body(definition_interface);

	void wsad_character_physics(definition_interface);
	void wsad_character_legs(definition_interface legs, definition_interface player);
	void wsad_character(definition_interface, definition_interface crosshair_entity);
	void wsad_character_corpse(definition_interface);

	void inject_window_input_to_character(definition_interface target_character, definition_interface camera_entity);

	void make_always_visible(definition_interface);
	void cancel_always_visible(definition_interface);

	void character_inventory(definition_interface);
	void backpack(definition_interface);

	void default_gun_container(definition_interface);
	void default_gun(definition_interface);

	void standard_pathfinding_capability(definition_interface);
	void soldier_intelligence(definition_interface);
}

namespace prefabs {
	entity_id create_car(cosmos&, components::transform);
	entity_id create_motorcycle(cosmos&, components::transform);

	entity_id create_sample_magazine(cosmos&, vec2 pos, std::string space = "0.30", entity_id charge_inside = entity_id());
	entity_id create_sample_suppressor(cosmos& world, vec2 pos);
	entity_id create_sample_rifle(cosmos&, vec2 pos, entity_id load_mag = entity_id());
	entity_id create_pistol(cosmos&, vec2 pos, entity_id load_mag = entity_id());
	entity_id create_submachine(cosmos&, vec2 pos, entity_id load_mag = entity_id());
	
	entity_id create_pink_charge(cosmos&, vec2 pos, int charges = 23);
	entity_id create_cyan_charge(cosmos&, vec2 pos, int charges = 30);
	entity_id create_green_charge(cosmos& world, vec2 pos, int charges = 23);

	entity_id create_sample_backpack(cosmos&, vec2 pos);

	entity_id create_character_crosshair(cosmos&);
	entity_id create_character(cosmos&, vec2 pos);
	entity_id create_crate(cosmos&, vec2 pos, vec2 size);

	entity_id create_cyan_urban_machete(cosmos&, vec2 pos);
}