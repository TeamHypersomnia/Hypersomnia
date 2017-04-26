#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/grenade_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/shape_polygon_component.h"

#include "game/enums/entity_name.h"
#include "game/enums/filters.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_force_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("force_grenade");

		auto& grenade = grenade_entity += components::grenade();
		
		grenade.type = adverse_element_type::FORCE;
		grenade.spoon = grenade_spoon;
		
		//{
		//	ingredients::add_sprite(grenade_spoon, pos, assets::game_image_id::GRENADE_SPOON, white, render_layer::SMALL_DYNAMIC_BODY);
		//	ingredients::add_shell_dynamic_body(grenade_spoon);
		//}

		name_entity(grenade_entity, entity_name::FORCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::FORCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity);

		grenade.released_image_id = assets::game_image_id::FORCE_GRENADE_RELEASED;

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);
		grenade_entity.get<components::processing>().disable_in(processing_subjects::WITH_GRENADE);

		return grenade_entity;
	}

	entity_handle create_ped_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("ped_grenade");
		auto& grenade = grenade_entity += components::grenade();

		grenade.type = adverse_element_type::PED;
		grenade.spoon = grenade_spoon;

		name_entity(grenade_entity, entity_name::PED_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::PED_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade.released_image_id = assets::game_image_id::PED_GRENADE_RELEASED;

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);

		return grenade_entity;
	}

	entity_handle create_interference_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("interference_grenade");
		auto& grenade = grenade_entity += components::grenade();

		grenade.type = adverse_element_type::INTERFERENCE;
		grenade.spoon = grenade_spoon;

		name_entity(grenade_entity, entity_name::INTERFERENCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::INTERFERENCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");
		
		grenade.released_image_id = assets::game_image_id::INTERFERENCE_GRENADE_RELEASED;

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);

		return grenade_entity;
	}
}

