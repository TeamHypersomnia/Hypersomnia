#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"
#include "game/assets/particle_effect_response_id.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/particle_effect_response_component.h"

#include "game/enums/entity_name.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_cyan_urban_machete(cosmos& world, vec2 pos) {
		auto& machete = world.create_entity("urban_cyan_machete");
		name_entity(machete, entity_name::URBAN_CYAN_MACHETE);

		auto& sprite = ingredients::add_sprite(machete, pos, assets::texture_id::URBAN_CYAN_MACHETE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(machete);

		auto& item = ingredients::make_item(machete);
		item.space_occupied_per_charge = to_space_units("2.5");

		auto& melee = machete += components::melee();

		melee.swings[0].cooldown_ms = 100.f;
		melee.swings[0].duration_ms = 600.f; 
		melee.swings[0].acceleration = 5.f;

		melee.swings[1].cooldown_ms = 100.f;
		melee.swings[1].duration_ms = 600.f;
		melee.swings[1].acceleration = 4.f;

		melee.swings[2].cooldown_ms = 100.f;
		melee.swings[2].duration_ms = 400.f;
		melee.swings[2].acceleration = 5.f;

		melee.swings[3].cooldown_ms = 100.f;
		melee.swings[3].duration_ms = 400.f;
		melee.swings[3].acceleration = 5.f;

		melee.swings[4].duration_ms = -1.f;
		melee.swings[4].acceleration = 1.f;
		melee.swings[4].cooldown_ms = -1.f;

		std::vector<vec2> circle = generate_circle_points(100, 90, 0, 20);
		float angle = -90.f;
		for (size_t i = 0;i < circle.size();++i) {
			circle[i].y -= 100;
			components::transform current;
			current.pos = circle[i];
			current.rotation = angle;
			melee.offset_positions[1].push_back(current);
			angle += 4.5f;
		}

		melee.offset_positions[0] = melee.offset_positions[1];
		std::reverse(std::begin(melee.offset_positions[0]), std::end(melee.offset_positions[0]));

		melee.offset_positions[2] = {
			{ vec2(0, 0), 0 },
			{ vec2(10, 0), 0 },
			{ vec2(20, 0), 0 },
			{ vec2(30, 0), 0 },
			{ vec2(40, 0), 0 },
			{ vec2(50, 0), 0 },
			{ vec2(64, 0), 0 },
			{ vec2(120, 0), 0 },
			{ vec2(150, 0), 0 },
			{ vec2(180, 0), 0 },
		};

		melee.offset_positions[3] = melee.offset_positions[2];
		std::reverse(std::begin(melee.offset_positions[3]), std::end(melee.offset_positions[3]));

		auto& damage = machete += components::damage();
		damage.destroy_upon_damage = false;
		damage.damage_upon_collision = false;
		damage.amount = 50.f;
		damage.impulse_upon_hit = 1000.f;
		damage.sender = machete;
		damage.constrain_distance = false;
		damage.constrain_lifetime = false;

		auto& response = machete += components::particle_effect_response{ assets::particle_effect_response_id::SWINGING_MELEE_WEAPON_RESPONSE };
		response.modifier.colorize = cyan;
		
		machete.add_standard_components();

		return machete;
	}
}

