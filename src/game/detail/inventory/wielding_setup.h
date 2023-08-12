#pragma once
#include <array>
#include "game/cosmos/entity_id.h"

#include "game/detail/inventory/hand_count.h"

template <class id_type>
struct basic_wielding_setup  {
	using this_type = basic_wielding_setup<id_type>;
	using hand_selections_array = std::array<id_type, hand_count_v>;

	// GEN INTROSPECTOR struct basic_wielding_setup class id_type
	hand_selections_array hand_selections;
	// END GEN INTROSPECTOR

	basic_wielding_setup() {
		hand_selections[0].unset();
		hand_selections[1].unset();

		hand_selections[0].raw.version = 1337;
		hand_selections[1].raw.version = 1337;
	}

	template <class E>
	static this_type from_current(const E& character_entity);

	static this_type bare_hands() {
		this_type result;

		for (auto& h : result.hand_selections) {
			h = id_type::dead();
		}

		return result;
	}

	bool is_set() const {
		return !(*this == this_type());
	}

	bool operator==(const this_type& b) const {
		return hand_selections == b.hand_selections;  
	}

	template <class C>
	std::size_t least_weapon_index(const C&) const;

	template <class E>
	id_type get_other_than(const E& wielded_item) const;

	template <class E>
	void clear_hand_with(const E& wielded_item);

	template <class E>
	auto make_viable_setup(const E& character_entity) const;

	template <class C>
	bool is_akimbo(const C& cosm) const;

	template <class C>
	bool is_bare_hands(const C& cosm) const;

	template <class C>
	bool equivalent_to(const C& cosm, const basic_wielding_setup<id_type>&) const;

	auto& switch_hands() {
		std::swap(hand_selections[0], hand_selections[1]);
		return *this;
	}

	template <class E>
	bool same_as_in(const E& character_entity) const {
		return *this == from_current(character_entity);
	}

	template <class C, class F>
	decltype(auto) on_more_recent_item(C& cosm, F&& callback) const;
};

using wielding_setup = basic_wielding_setup<entity_id>;
using signi_wielding_setup = basic_wielding_setup<signi_entity_id>;
