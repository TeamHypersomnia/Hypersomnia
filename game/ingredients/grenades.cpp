#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"
#include "game/assets/particle_effect_response_id.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/particle_effect_response_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/grenade_component.h"

#include "game/enums/entity_name.h"
#include "game/enums/filters.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_force_grenade(cosmos& world, vec2 pos) {
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("force_grenade");
		auto& grenade = grenade_entity += components::grenade();
		
		grenade.type = grenade_type::FORCE;
		grenade.spoon = grenade_spoon;
		//{
		//	auto& sprite = grenade_spoon += components::sprite();
		//	auto& render = grenade_spoon += components::render();
		//
		//	components::fixtures colliders;
		//
		//	render.layer = render_layer::SMALL_DYNAMIC_BODY;
		//
		//	sprite.set(assets::game_image_id::GRENADE_SPOON);
		//
		//	auto& fixture = colliders.new_collider();
		//
		//	fixture.shape.from_renderable(grenade_spoon);
		//	fixture.density = 0.6f;
		//	fixture.filter = filters::renderable();
		//	fixture.sensor = true;
		//
		//	vec2 offset(-10, -10);
		//	colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
		//
		//	grenade_spoon += colliders;
		//
		//	grenade_spoon.get<components::fixtures>().set_owner_body(grenade);
		//}

		name_entity(grenade_entity, entity_name::FORCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::FORCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade_entity);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade_entity.add_standard_components();

		return grenade_entity;
	}

	entity_handle create_ped_grenade(cosmos& world, vec2 pos) {
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("ped_grenade");
		auto& grenade = grenade_entity += components::grenade();

		grenade.type = grenade_type::PED;
		grenade.spoon = grenade_spoon;

		name_entity(grenade_entity, entity_name::PED_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::PED_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade_entity);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade_entity.add_standard_components();

		return grenade_entity;
	}

	entity_handle create_interference_grenade(cosmos& world, vec2 pos) {
		const auto grenade_spoon = world.create_entity("grenade_spoon");
		const auto grenade_entity = world.create_entity("interference_grenade");
		auto& grenade = grenade_entity += components::grenade();

		grenade.type = grenade_type::INTERFERENCE;
		grenade.spoon = grenade_spoon;

		name_entity(grenade_entity, entity_name::INTERFERENCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade_entity, pos, assets::game_image_id::INTERFERENCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade_entity);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade_entity.add_standard_components();

		return grenade_entity;
	}
}

