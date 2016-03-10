#include "inventory_utils.h"
#include "entity_system/entity.h"
#include "game_framework/components/item_component.h"

augs::entity_id get_owning_transfer_capability(augs::entity_id entity) {
	if (entity.dead())
		return augs::entity_id();

	auto* maybe_transfer_capability = entity->find<components::item_slot_transfers>();

	if (maybe_transfer_capability)
		return entity;

	auto* maybe_item = entity->find<components::item>();

	if (!maybe_item || maybe_item->current_slot.dead())
		return augs::entity_id();

	return get_owning_transfer_capability(maybe_item->current_slot.container_entity);
}

inventory_slot_id first_free_hand(augs::entity_id root_container) {
	auto maybe_primary = root_container[slot_function::PRIMARY_HAND];
	auto maybe_secondary = root_container[slot_function::SECONDARY_HAND];

	if (maybe_primary.is_empty_slot())
		return maybe_primary;

	if (maybe_secondary.is_empty_slot())
		return maybe_secondary;

	return inventory_slot_id();
}

inventory_slot_id determine_hand_holstering_slot(augs::entity_id item_entity, augs::entity_id searched_root_container) {
	assert(item_entity.alive());
	assert(searched_root_container.alive());

	auto maybe_shoulder = searched_root_container[slot_function::SHOULDER_SLOT];

	if (maybe_shoulder.alive()) {
		if (maybe_shoulder.can_contain(item_entity))
			return maybe_shoulder;
		else if (maybe_shoulder->items_inside.size() > 0) {
			auto maybe_item_deposit = maybe_shoulder->items_inside[0][slot_function::ITEM_DEPOSIT];

			if (maybe_item_deposit.alive() && maybe_item_deposit.can_contain(item_entity))
				return maybe_item_deposit;
		}
	}
	else {
		auto maybe_armor = searched_root_container[slot_function::TORSO_ARMOR_SLOT];

		if (maybe_armor.alive())
			if (maybe_armor.can_contain(item_entity))
				return maybe_armor;
	}

	return inventory_slot_id();
}

inventory_slot_id determine_pickup_target_slot(augs::entity_id item_entity, augs::entity_id searched_root_container) {
	assert(item_entity.alive());
	assert(searched_root_container.alive());

	auto hidden_slot = determine_hand_holstering_slot(item_entity, searched_root_container);;

	if (hidden_slot.alive())
		return hidden_slot;

	auto hand = first_free_hand(searched_root_container);

	if (hand.can_contain(item_entity))
		return hand;

	return inventory_slot_id();
}

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(augs::entity_id root_container, int is_action_secondary) {
	inventory_slot_id subject_hand;
	subject_hand.container_entity = root_container;

	auto primary = root_container[slot_function::PRIMARY_HAND];
	auto secondary = root_container[slot_function::SECONDARY_HAND];

	if (primary.is_empty_slot())
		return secondary;
	else
		return is_action_secondary ? secondary : primary;
}

item_transfer_result query_transfer_result(messages::item_slot_transfer_intent r) {
	item_transfer_result predicted_result;

	auto& item = r.item->get<components::item>();

	auto item_owning_capability = get_owning_transfer_capability(r.item);
	auto target_slot_owning_capability = get_owning_transfer_capability(r.target_slot.container_entity);

	assert(item_owning_capability.alive() || target_slot_owning_capability.alive());

	if (item_owning_capability.alive() && target_slot_owning_capability.alive() &&
		item_owning_capability != target_slot_owning_capability)
		predicted_result = INVALID_SLOT_OR_UNOWNED_ROOT;
	else if (r.target_slot.alive())
		predicted_result = r.target_slot.containment_result(r.item);

	if (predicted_result == item_transfer_result::SUCCESSFUL_TRANSFER) {
		if (item.current_mounting == components::item::MOUNTED)
			predicted_result = item_transfer_result::UNMOUNT_BEFOREHAND;
	}

	return predicted_result;
}

std::pair<item_transfer_result, slot_function> query_transfer_result(augs::entity_id from, augs::entity_id to) {
	auto* container = to->find<components::container>();

	auto most_meaningful_error = item_transfer_result::NO_SLOT_AVAILABLE;

	if (container) {
		if (to[slot_function::ITEM_DEPOSIT].alive())
			return{ query_transfer_result({ from, to[slot_function::ITEM_DEPOSIT] }), slot_function::ITEM_DEPOSIT };

		for (auto& s : container->slots) {
			auto res = query_transfer_result({ from, to[s.first] });
			
			if (res >= item_transfer_result::SUCCESSFUL_TRANSFER) {
				return{ res, s.first };
			}
			else
				most_meaningful_error = std::max(most_meaningful_error, res);
		}
	}

	return{ most_meaningful_error, slot_function::INVALID };
}
