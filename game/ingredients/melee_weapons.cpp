#include "ingredients.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "game/assets/particle_effect.h"
#include "game/assets/particle_effect_response.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/particle_effect_response_component.h"

#include "game/globals/entity_name.h"

#include "game/detail/inventory_utils.h"

namespace prefabs {
	augs::entity_id create_cyan_urban_machete(augs::world& world, vec2 pos) {
		auto& machete = world.create_entity("urban_cyan_machete");
		name_entity(machete, entity_name::URBAN_CYAN_MACHETE);

		auto& sprite = ingredients::sprite(machete, pos, assets::texture_id::URBAN_CYAN_MACHETE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::crate_physics(machete);

		auto& item = ingredients::make_item(machete);
		item.space_occupied_per_charge = to_space_units("2.5");

		auto& melee = *machete += components::melee();
		melee.swing_cooldown_ms = 50.f;
		melee.swing_duration_ms = 200.f;
		melee.swing_acceleration = 5.f;
		//std::vector<vec2> circle = generate_circle_points(50, 90, 0, 50);
		//double angle = 0;
		//for (int i = 0;i < circle.size();++i) {
		//	//circle[i].y -= 100;
		//	components::transform current;
		//	current.pos = circle[i];
		//	current.rotation = angle;
		//	melee.offset_positions.push_back(current);
		//	angle += 4.5;
		//}

		melee.offset_positions = {
			{ vec2(0, 0), 0 },
			{ vec2(10, 0), -2 },
			{ vec2(20, 0), -10 },
			{ vec2(30, -10), -14 },
			{ vec2(40, -20), -16 },
			{ vec2(50, -25), -20 },
			{ vec2(64, -34), -30 },
			{ vec2(120, -54), -45 },
		};

		auto& damage = *machete += components::damage();
		damage.destroy_upon_damage = false;
		damage.damage_upon_collision = false;
		damage.amount = 50.f;
		damage.impulse_upon_hit = 1000.f;
		damage.sender = machete;
		damage.constrain_distance = false;
		damage.constrain_lifetime = false;

		auto& response = *machete += components::particle_effect_response{ assets::particle_effect_response_id::SWINGING_MELEE_WEAPON_RESPONSE };
		response.modifier.colorize = augs::cyan;
		
		return machete;
	}
}

