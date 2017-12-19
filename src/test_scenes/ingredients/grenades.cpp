#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/ids/particle_effect_id.h"

#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/explosive_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sender_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/enums/filters.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_force_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto& metas = step.input.logical_assets;
		const auto grenade_entity = world.create_entity("Force grenade");

		auto& sender = grenade_entity += components::sender();
		auto& explosive = grenade_entity += components::explosive();
		auto& fuse = grenade_entity += components::hand_fuse();
		fuse.throw_sound.id = assets::sound_buffer_id::GRENADE_THROW;
		fuse.unpin_sound.id = assets::sound_buffer_id::GRENADE_UNPIN;

		auto& in = explosive.explosion;
		in.type = adverse_element_type::FORCE;
		in.damage = 88.f;
		in.inner_ring_color = red;
		in.outer_ring_color = orange;
		in.effective_radius = 300.f;
		in.impact_force = 550.f;
		in.sound_gain = 1.8f;
		in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

		auto& sprite = ingredients::add_sprite(metas, grenade_entity, assets::game_image_id::FORCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity, pos);

		explosive.released_image_id = assets::game_image_id::FORCE_GRENADE_RELEASED;
		explosive.released_physical_material = assets::physical_material_id::GRENADE;

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);
		
		auto& meta = grenade_entity.get_meta_of_name();

		meta.description =
			L"Throwable explosive with a three seconds delay.\nDeals damage to [color=red]Health[/color]."
		;

		return grenade_entity;
	}

	entity_handle create_ped_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto& metas = step.input.logical_assets;
		const auto grenade_entity = world.create_entity("PED grenade");

		auto& sender = grenade_entity += components::sender();
		auto& explosive = grenade_entity += components::explosive();
		auto& fuse = grenade_entity += components::hand_fuse();
		fuse.throw_sound.id = assets::sound_buffer_id::GRENADE_THROW;
		fuse.unpin_sound.id = assets::sound_buffer_id::GRENADE_UNPIN;

		auto& in = explosive.explosion;
		in.damage = 88.f;
		in.inner_ring_color = cyan;
		in.outer_ring_color = turquoise;
		in.effective_radius = 350.f;
		in.impact_force = 20.f;
		in.sound_gain = 2.2f;
		in.sound_effect = assets::sound_buffer_id::PED_EXPLOSION;
		in.type = adverse_element_type::PED;
		in.create_thunders_effect = true;

		auto& sprite = ingredients::add_sprite(metas, grenade_entity, assets::game_image_id::PED_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity, pos);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");

		explosive.released_image_id = assets::game_image_id::PED_GRENADE_RELEASED;
		explosive.released_physical_material = assets::physical_material_id::GRENADE;

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);

		auto& meta = grenade_entity.get_meta_of_name();

		meta.description =
			L"Throwable explosive with a three seconds delay.\nDrains [color=cyan]Personal Electricity[/color].\nIf the subject has [color=turquoise]Electric Shield[/color] enabled,\nthe effect is doubled."
		;

		return grenade_entity;
	}

	entity_handle create_interference_grenade(const logic_step step, vec2 pos) {
		auto& world = step.cosm;
		const auto& metas = step.input.logical_assets;
		const auto grenade_entity = world.create_entity("Interference grenade");
		
		auto& sender = grenade_entity += components::sender();
		auto& explosive = grenade_entity += components::explosive();
		auto& fuse = grenade_entity += components::hand_fuse();
		fuse.throw_sound.id = assets::sound_buffer_id::GRENADE_THROW;
		fuse.unpin_sound.id = assets::sound_buffer_id::GRENADE_UNPIN;

		auto& in = explosive.explosion;

		in.damage = 100.f;
		in.inner_ring_color = yellow;
		in.outer_ring_color = orange;
		in.effective_radius = 450.f;
		in.impact_force = 20.f;
		in.sound_gain = 2.2f;
		in.sound_effect = assets::sound_buffer_id::INTERFERENCE_EXPLOSION;
		in.type = adverse_element_type::INTERFERENCE;

		auto& sprite = ingredients::add_sprite(metas, grenade_entity, assets::game_image_id::INTERFERENCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, grenade_entity, pos);

		auto& item = ingredients::make_item(grenade_entity);
		item.space_occupied_per_charge = to_space_units("0.6");
		
		explosive.released_image_id = assets::game_image_id::INTERFERENCE_GRENADE_RELEASED;
		explosive.released_physical_material = assets::physical_material_id::GRENADE;

		components::shape_circle shape_circle;
		shape_circle.radius = 1.f;

		grenade_entity += shape_circle;

		grenade_entity.add_standard_components(step);

		auto& meta = grenade_entity.get_meta_of_name();

		meta.description =
			L"Throwable explosive with a three seconds delay.\nDeals damage to [color=orange]Consciousness[/color].\nCauses massive aimpunch."
		;

		return grenade_entity;
	}
}

