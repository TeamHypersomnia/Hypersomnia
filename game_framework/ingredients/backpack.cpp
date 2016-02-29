#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/container_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/trigger_component.h"
#include "game_framework/components/force_joint_component.h"

namespace ingredients {
	void backpack(augs::entity_id e) {
		auto& container = *e += components::container();
		auto item = make_item(e);
		components::force_joint force_joint;
		
		inventory_slot slot_def;
		slot_def.space_available = 7;

		container.slots[slot_function::ITEM_DEPOSIT] = slot_def;

		item.dual_wield_accuracy_loss_multiplier = 1;
		item.dual_wield_accuracy_loss_percentage = 50;
		item.space_occupied_per_charge = 1;
		item.categories_for_slot_compatibility = item_category::SHOULDER_CONTAINER;

		e->add(force_joint);
		e->disable(force_joint);
	}
}

namespace prefabs {
	augs::entity_id create_sample_backpack(augs::world& world, vec2 pos) {
		auto sample_backpack = world.create_entity("sample_backpack");
		
		ingredients::backpack(sample_backpack);

		ingredients::sprite(sample_backpack, pos, assets::texture_id::BACKPACK, augs::white, render_layer::DROPPED_ITEM);
		ingredients::crate_physics(sample_backpack);

		return sample_backpack;
	}
}