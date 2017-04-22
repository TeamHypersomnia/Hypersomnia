#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/trigger_collision_detector_component.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/enums/entity_name.h"

namespace ingredients {
	void add_character_head_inventory(const logic_step step, entity_handle e) {
		auto& container = e += components::container();
		auto& item_slot_transfers = e += components::item_slot_transfers();
		auto& detector = e += components::trigger_collision_detector();

		const auto bbox = e.get_aabb(step.input.metas_of_assets, components::transform{}).get_size();

		{
			inventory_slot slot_def;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_BODIES_BY_JOINT;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = vec2(bbox.x / 2 - 3, 20);
			slot_def.category_allowed = item_category::ARM_BACK;
			container.slots[slot_function::PRIMARY_ARM_BACK] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_BODIES_BY_JOINT;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = vec2(bbox.x / 2 - 3, -20);
			slot_def.category_allowed = item_category::ARM_BACK;
			container.slots[slot_function::SECONDARY_ARM_BACK] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.category_allowed = item_category::SHOULDER_CONTAINER;
			slot_def.physical_behaviour = slot_physical_behaviour::MAKE_BODIES_FIXTURES;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::LEFT;
			slot_def.attachment_offset.pos = vec2(-bbox.x / 2 + 4, 0);
			container.slots[slot_function::SHOULDER] = slot_def;
		}
	}
}

namespace prefabs {
	entity_handle create_standard_arm_back(
		const logic_step step,
		const vec2 size
	) {
		auto e = step.cosm.create_entity("arm_back");
		name_entity(e, entity_name::STANDARD_ARM_BACK);

		auto& container = e += components::container();
		auto& sprite = e += components::sprite();

		sprite.set(assets::game_image_id::BLANK, size, orange);

		auto& render = e += components::render();

		render.layer = render_layer::SMALL_DYNAMIC_BODY;

		ingredients::add_see_through_dynamic_body(step, e); 
		
		auto& item = ingredients::make_item(e);
		item.categories_for_slot_compatibility.set(item_category::ARM_BACK);

		{
			inventory_slot slot_def;
			slot_def.category_allowed = item_category::ARM_FRONT;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_BODIES_BY_JOINT;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = vec2(size.x / 2, 0);
			container.slots[slot_function::ARM_FRONT] = slot_def;
		}

		return e.add_standard_components(step);
	}

	entity_handle create_standard_arm_front(
		const logic_step step,
		const vec2 size
	) {
		auto e = step.cosm.create_entity("arm_front");
		name_entity(e, entity_name::STANDARD_ARM_FRONT);

		auto& container = e += components::container();
		auto& sprite = e += components::sprite();

		sprite.set(assets::game_image_id::BLANK, size, red);

		auto& render = e += components::render();

		render.layer = render_layer::SMALL_DYNAMIC_BODY;

		ingredients::add_see_through_dynamic_body(step, e);

		auto& item = ingredients::make_item(e);
		item.categories_for_slot_compatibility.set(item_category::ARM_FRONT);

		{
			inventory_slot slot_def;
			slot_def.category_allowed = item_category::GENERAL;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_BODIES_BY_JOINT;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = vec2(size.x / 2, 0);
			container.slots[slot_function::WIELDED_ITEM] = slot_def;
		}

		return e.add_standard_components(step);
	}

	entity_handle create_sample_complete_arm(
		const logic_step step,
		const vec2 back_arm_size,
		const vec2 front_arm_size
	) {
		const auto back = create_standard_arm_back(step, back_arm_size);
		const auto front = create_standard_arm_front(step, front_arm_size);

		item_slot_transfer_request r;
		r.item = front;
		r.target_slot = back[slot_function::ARM_FRONT];
		perform_transfer(r, step);

		return back;
	}
}