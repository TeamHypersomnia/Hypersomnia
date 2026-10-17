#pragma once
#include <algorithm>
#include <cstdint>
#include "augs/templates/reversion_wrapper.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/inventory/wears_backpack.h"

/*
	Firearms are carried only in hands or hung on the back - never inside containers.
	A character carries two at most, three with a backpack,
	so holstering always finds a free slot on the back for every firearm in hands.
*/

constexpr int max_firearms_without_backpack_v = 2;

inline bool is_firearm_like(const item_category_flagset& categories) {
	return categories.test(item_category::BACK_WEAPON);
}

template <class E>
bool is_firearm_like(const E& handle) {
	if (const auto item = handle.template find<invariants::item>()) {
		return ::is_firearm_like(item->categories_for_slot_compatibility);
	}

	return false;
}

template <class E>
int calc_max_firearms(const E& character) {
	return max_firearms_without_backpack_v + (::wears_backpack(character) ? 1 : 0);
}

template <class E, class F>
void for_each_carried_firearm(const E& character, F&& callback) {
	auto check = [&](const slot_function type) {
		if (const auto item = character[type].get_item_if_any()) {
			if (::is_firearm_like(item)) {
				callback(item);
			}
		}
	};

	check(slot_function::PRIMARY_HAND);
	check(slot_function::SECONDARY_HAND);

	for (const auto back_slot : back_weapon_slots_v) {
		check(back_slot);
	}
}

template <class E>
int count_carried_firearms(const E& character, const entity_id except = entity_id()) {
	auto n = 0;

	::for_each_carried_firearm(character, [&](const auto& firearm) {
		if (entity_id(firearm.get_id()) != except) {
			++n;
		}
	});

	return n;
}

/*
	Picking up a firearm over the limit swaps it for one we carry.
	With pickup_swaps_wielded_firearm (the default) the one in hands - we "draw" what we want to throw away -
	or, with no firearm in hands, the one at the very bottom of the back.
	Otherwise the one on top of the back, so picking up never throws away what we hold
	(with no firearm in hands, the picked one is still drawn - see wield_picked_firearm_if_hands_hold_none).
	Either way, the other kind is taken when there is none of the preferred one, e.g. both firearms in hands.
*/

/*
	All weapons on the back hang in the same place, stacked in the order they were hidden:
	the one hidden longest ago at the bottom, the most recent on top - drawn last.
	Ties keep the order of the slots. Empty slots come first.
*/

template <class E>
auto get_back_weapon_slots_bottom_to_top(const E& character) {
	auto ordered = back_weapon_slots_v;

	auto when_stowed = [&](const slot_function type) {
		if (const auto item = character[type].get_item_if_any()) {
			return item.when_last_transferred().step;
		}

		return static_cast<uint32_t>(0);
	};

	std::stable_sort(
		ordered.begin(),
		ordered.end(),
		[&](const slot_function a, const slot_function b) {
			return when_stowed(a) < when_stowed(b);
		}
	);

	return ordered;
}

template <class E>
entity_id find_top_firearm_on_back(const E& character) {
	const auto bottom_to_top = ::get_back_weapon_slots_bottom_to_top(character);

	for (const auto back_slot : reverse(bottom_to_top)) {
		if (const auto item = character[back_slot].get_item_if_any()) {
			return item.get_id();
		}
	}

	return entity_id();
}

template <class E>
entity_id find_bottom_firearm_on_back(const E& character) {
	for (const auto back_slot : ::get_back_weapon_slots_bottom_to_top(character)) {
		if (const auto item = character[back_slot].get_item_if_any()) {
			return item.get_id();
		}
	}

	return entity_id();
}

template <class E>
entity_id find_wielded_firearm_to_swap(const E& character) {
	for (const auto hand : { slot_function::SECONDARY_HAND, slot_function::PRIMARY_HAND }) {
		if (const auto item = character[hand].get_item_if_any()) {
			if (::is_firearm_like(item)) {
				return item.get_id();
			}
		}
	}

	return entity_id();
}

template <class E, class I>
entity_id find_firearm_to_swap_for(const E& character, const I& picked_item) {
	if (!::is_firearm_like(picked_item)) {
		return entity_id();
	}

	if (::count_carried_firearms(character) < ::calc_max_firearms(character)) {
		return entity_id();
	}

	const bool prefer_wielded = [&]() {
		if (const auto transfers = character.template find<components::item_slot_transfers>()) {
			return transfers->pickup_swaps_wielded_firearm;
		}

		return false;
	}();

	const auto on_back = ::find_top_firearm_on_back(character);
	const auto wielded = ::find_wielded_firearm_to_swap(character);

	if (prefer_wielded) {
		/*
			With no firearm in hands, the one at the very bottom of the back (hidden the longest ago) goes -
			so a quick H before picking up keeps the held one and swaps the older one.
		*/

		return wielded.is_set() ? wielded : ::find_bottom_firearm_on_back(character);
	}

	return on_back.is_set() ? on_back : wielded;
}
