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

		melee.swing_cooldown_ms[0] = 100.f;
		melee.swing_duration_ms[0] = 400.f;
		melee.swing_acceleration[0] = 5.f;

		melee.swing_cooldown_ms[1] = 100.f;
		melee.swing_duration_ms[1] = 600.f;
		melee.swing_acceleration[1] = 4.f;

		melee.swing_cooldown_ms[2] = 100.f;
		melee.swing_duration_ms[2] = 400.f;
		melee.swing_acceleration[2] = 5.f;

		melee.swing_cooldown_ms[3] = 100.f;
		melee.swing_duration_ms[3] = 400.f;
		melee.swing_acceleration[3] = 5.f;

		melee.swing_duration_ms[4] = -1.f;

		std::vector<vec2> circle = generate_circle_points(100, 90, 0, 20);
		double angle = -90;
		for (int i = 0;i < circle.size();++i) {
			circle[i].y -= 100;
			components::transform current;
			current.pos = circle[i];
			current.rotation = angle;
			melee.offset_positions[1].push_back(current);
			angle += 4.5;
		}

		melee.offset_positions[0] = melee.offset_positions[1];
		std::reverse(std::begin(melee.offset_positions[0]), std::end(melee.offset_positions[0]));

		melee.offset_positions[2] = {
			{ vec2(-20, 0) + vec2(0, 0), 0 },
			{ vec2(-20, 0) + vec2(0, 10), 10 },
			{ vec2(-20, 0) + vec2(10, 30), 30 },
			{ vec2(-20, 0) + vec2(-30, 50), 60 },
			{ vec2(-20, 0) + vec2(10, -10), 0 },
			{ vec2(-20, 0) + vec2(15, -25), 0 },
			{ vec2(-20, 0) + vec2(20, -45), 0 },
			{ vec2(-20, 0) + vec2(10, -60), 0 },
			{ vec2(-20, 0) + vec2(-10, -70), -30 },
			{ vec2(-20, 0) + vec2(-20, -75), -60 },
			{ vec2(-20, 0) + vec2(-30, -80), -70 },
			{ vec2(-20, 0) + vec2(-40, -97), -90 },
			{ vec2(-20, 0) + vec2(-50, -104), -100 },
			{ vec2(-20, 0) + vec2(-80, -119), -120 },
		};

		melee.offset_positions[3] = melee.offset_positions[2];
		std::reverse(std::begin(melee.offset_positions[3]), std::end(melee.offset_positions[3]));

		melee.offset_positions[0][0] = vec2(-20, 0);
		melee.offset_positions[1][0] = vec2(-20, 0);
		melee.offset_positions[2][0] = vec2(-20, 0);
		melee.offset_positions[3][0] = vec2(-20, 0);

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

