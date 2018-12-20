#pragma once
#include <optional>
#include "augs/templates/maybe_const.h"
#include "augs/misc/enum/enum_boolset.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/slot_physical_behaviour.h"

#include "game/detail/physics/colliders_connection.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_types.h"
#include "game/components/transform_component.h"

#include "game/detail/inventory/inventory_slot_types.h"

struct inventory_slot;
inventory_space_type calc_space_occupied_with_children(const_entity_handle item);

class cosmos;

template <class entity_handle_type>
class basic_inventory_slot_handle {
	static constexpr bool is_const = is_handle_const_v<entity_handle_type>;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;

	/* Inventory slots are invariants, thus always const. */
	using slot_reference = const inventory_slot&;
	using slot_pointer = const inventory_slot*;

public:
	basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id);
	
	owner_reference get_cosmos() const;

	owner_reference owner;
	inventory_slot_id raw_id;

	void unset();

	slot_reference get() const;
	slot_reference operator*() const;
	slot_pointer operator->() const;

	bool alive() const;
	bool dead() const;

	bool can_contain(const entity_id) const;
	bool can_contain_whole(const entity_id) const;

	entity_handle_type get_item_if_any() const;
	entity_handle_type get_container() const;
	slot_function get_type() const;

	entity_handle_type get_root_container() const;
	entity_handle_type get_root_container_until(const entity_id container_entity) const;

	bool is_child_of(const entity_id container_entity) const;

	const std::vector<entity_id>& get_items_inside() const;

	bool has_items() const;
	bool is_empty_slot() const;
	bool is_personal_item_deposit() const;

	bool is_hand_slot() const;
	size_t get_hand_index() const;

	float calc_density_multiplier_due_to_being_attached() const;

	inventory_space_type calc_local_space_available() const;
	inventory_space_type calc_space_occupied_by_children() const;
	inventory_space_type calc_real_space_available() const;

	bool is_physically_reachable_from(
		const entity_id until_parent,
	   	const optional_slot_flags& bypass_slots = std::nullopt
	) const;

	bool is_ancestor_mounted() const;
	bool is_this_or_ancestor_mounted() const;

	inventory_slot_id get_id() const;
	operator inventory_slot_id() const;

	operator basic_inventory_slot_handle<typename entity_handle_type::const_type>() const {
		return { owner, raw_id };
	}

	explicit operator bool() const {
		return alive();
	}
};

template <class C>
auto subscript_handle_getter(C& cosm, const inventory_slot_id id) 
	-> basic_inventory_slot_handle<basic_entity_handle<std::is_const_v<C>>>
{
	return { cosm, id };
}

template <class E>
const auto& get_items_inside(const E h, const slot_function s) {
	return h.get_cosmos()[inventory_slot_id{s, h.get_id()}].get_items_inside();  
}

template <class E>
inline basic_inventory_slot_handle<E>::basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id) : owner(owner), raw_id(raw_id) {}

template <class E>
inline typename basic_inventory_slot_handle<E>::owner_reference basic_inventory_slot_handle<E>::get_cosmos() const {
	return owner;
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_pointer basic_inventory_slot_handle<E>::operator->() const {
	return &get_container().template get<invariants::container>().slots.at(raw_id.type);
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_reference basic_inventory_slot_handle<E>::operator*() const {
	return *operator->();
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_reference basic_inventory_slot_handle<E>::get() const {
	return *operator->();
}

template <class E>
inline bool basic_inventory_slot_handle<E>::alive() const {
	if (get_container().dead()) {
		return false;
	}

	const auto* const container = get_container().template find<invariants::container>();

	return container && container->slots.find(raw_id.type) != container->slots.end();
}

template <class E>
inline bool basic_inventory_slot_handle<E>::dead() const {
	return !alive();
}

template <class E>
inline inventory_slot_id basic_inventory_slot_handle<E>::get_id() const {
	return raw_id;
}

template <class E>
inline basic_inventory_slot_handle<E>::operator inventory_slot_id() const {
	return get_id();
}

template <class E>
void basic_inventory_slot_handle<E>::unset() {
	raw_id.unset();
}

template <class E>
bool basic_inventory_slot_handle<E>::is_hand_slot() const {
	return get_hand_index() != 0xdeadbeef;
}

template <class E>
std::size_t basic_inventory_slot_handle<E>::get_hand_index() const {
	std::size_t index = 0xdeadbeef;

	if (raw_id.type == slot_function::PRIMARY_HAND) {
		index = 0;
	}
	else if (raw_id.type == slot_function::SECONDARY_HAND) {
		index = 1;
	}

	return index;
}

template <class E>
bool basic_inventory_slot_handle<E>::has_items() const {
	return get_items_inside().size() > 0;
}

template <class E>
E basic_inventory_slot_handle<E>::get_item_if_any() const {
	return get_cosmos()[(has_items() ? (*this).get_items_inside()[0] : entity_id())];
}

template <class E>
bool basic_inventory_slot_handle<E>::is_empty_slot() const {
	return get_items_inside().size() == 0;
}

template <class E>
bool basic_inventory_slot_handle<E>::is_personal_item_deposit() const {
	return get_type() == slot_function::ITEM_DEPOSIT && get_container().template has<components::item_slot_transfers>();
}

template <class E>
bool basic_inventory_slot_handle<E>::is_this_or_ancestor_mounted() const {
	if (dead()) {
		return false;
	}

	if (get().is_mounted_slot()) {
		return true;
	}

	return is_ancestor_mounted();
}

template <class E>
bool basic_inventory_slot_handle<E>::is_ancestor_mounted() const {
	if (dead()) {
		return false;
	}

	const auto parent = get_container().get_current_slot();

	if (parent.alive()) {
		if (parent->is_mounted_slot()) {
			return true;
		}

		return parent.is_ancestor_mounted();
	}

	return false;
}

template <class E>
bool basic_inventory_slot_handle<E>::is_physically_reachable_from(
	const entity_id from_parent,
	const optional_slot_flags& bypass_slots
) const {
	const bool passes_filter = bypass_slots != std::nullopt && bypass_slots->test(get_type());
	const bool should_item_here_keep_physical_body = passes_filter || get().makes_physical_connection();
	const bool reachable = should_item_here_keep_physical_body && !get().never_reachable_for_mounting;

	if (get_container() == from_parent) {
		return reachable;
	}

	const auto maybe_item = get_container().template find<components::item>();

	if (maybe_item) {
		const auto slot = owner[maybe_item->get_current_slot()];

		if (slot.alive()) {
			return std::min(reachable, slot.is_physically_reachable_from(from_parent, bypass_slots));
		}
	}

	return reachable;
}

template <class E>
float basic_inventory_slot_handle<E>::calc_density_multiplier_due_to_being_attached() const {
	const auto density_multiplier = get().attachment_density_multiplier;

	if (const auto maybe_item = get_container().template find<components::item>()) {
		if (const auto slot = owner[maybe_item->get_current_slot()]) {
			if (slot->physical_behaviour == slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY) {
				return density_multiplier * slot.calc_density_multiplier_due_to_being_attached();
			}

			return density_multiplier;
		}
	}

	return density_multiplier;
}

template <class E>
E basic_inventory_slot_handle<E>::get_root_container() const {
	const auto slot = get_container().get_current_slot();

	if (slot.alive()) {
		return slot.get_root_container();
	}

	return get_container();
}

template <class E>
E basic_inventory_slot_handle<E>::get_root_container_until(const entity_id container_entity) const {
	const auto slot = get_container().get_current_slot();

	if (slot.dead()) {
		return get_container();
	}

	if (slot.get_container() == container_entity) {
		return get_cosmos()[container_entity];
	}

	return slot.get_root_container_until(container_entity);
}

template <class E>
bool basic_inventory_slot_handle<E>::is_child_of(const entity_id container_entity) const {
	return get_container() == container_entity || get_root_container_until(container_entity) == container_entity;
}

template <class E>
E basic_inventory_slot_handle<E>::get_container() const {
	return get_cosmos()[raw_id.container_entity];
}

template <class E>
slot_function basic_inventory_slot_handle<E>::get_type() const {
	return raw_id.type;
}

template <class E>
inventory_space_type basic_inventory_slot_handle<E>::calc_real_space_available() const {
	/* 
		Special case: 
		maybe report 0 space available if there're items in the chamber and chamber magazine at the same time 
	*/

	if (get_type() == slot_function::GUN_CHAMBER_MAGAZINE) {
		const auto gun_container = get_container();

		if (const auto gun = gun_container.template find<invariants::gun>()) {
			if (!gun->allow_charge_in_chamber_magazine_when_chamber_loaded) {
				if (const auto chamber = gun_container[slot_function::GUN_CHAMBER]) {
					if (chamber.has_items()) {
						return 0;
					}
				}
			}
		}
	}

	const auto lsa = calc_local_space_available();

	const auto maybe_item = get_container().template find<components::item>();

	if (maybe_item != nullptr && get_cosmos()[maybe_item->get_current_slot()].alive()) {
		return std::min(lsa, get_cosmos()[maybe_item->get_current_slot()].calc_real_space_available());
	}

	return lsa;
}

template <class E>
bool basic_inventory_slot_handle<E>::can_contain(const entity_id id) const {
	if (dead()) {
		return false;
	}

	return query_containment_result(get_cosmos()[id], *this).transferred_charges > 0;
}

template <class E>
bool basic_inventory_slot_handle<E>::can_contain_whole(const entity_id id) const {
	if (dead()) {
		return false;
	}

	const auto item_handle = get_cosmos()[id];
	return static_cast<int>(query_containment_result(item_handle, *this).transferred_charges) == item_handle.get_charges();
}

template <class E>
inventory_space_type basic_inventory_slot_handle<E>::calc_space_occupied_by_children() const {
	inventory_space_type total = 0;

	for (const auto& e : get_items_inside()) {
		total += calc_space_occupied_with_children(get_cosmos()[e]);
	}

	return total;
}

template <class E>
inventory_space_type basic_inventory_slot_handle<E>::calc_local_space_available() const {
	if (get().has_unlimited_space()) {
		return max_inventory_space_v;
	}

	const auto space = get().space_available;
	const auto children_occupied = calc_space_occupied_by_children();
	
	return space > children_occupied ? space - children_occupied : 0;
}

template <class E>
const std::vector<entity_id>& basic_inventory_slot_handle<E>::get_items_inside() const {
	return get_cosmos().get_solvable_inferred().relational.get_items_of_slots().get_children_of(get_id());
}

template <class E>
std::ostream& operator<<(std::ostream& out, const basic_inventory_slot_handle<E> &x) {
	if (x.dead()) {
		return out << "(dead slot handle)";
	}

	return out << typesafe_sprintf("%x (%x)", x.get_type(), x.get_container());
}