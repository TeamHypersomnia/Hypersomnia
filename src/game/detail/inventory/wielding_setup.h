#pragma once
#include <array>
#include "augs/string/pretty_print.h"
#include "game/cosmos/entity_id.h"

static constexpr std::size_t hand_count_v = 2;

template <class id_type>
struct basic_wielding_setup  {
	using this_type = basic_wielding_setup<id_type>;
	using hand_selections_array = std::array<id_type, hand_count_v>;

	// GEN INTROSPECTOR struct basic_wielding_setup class id_type
	hand_selections_array hand_selections;
	// END GEN INTROSPECTOR

	template <class E>
	static this_type from_current(const E& character_entity);

	bool operator==(const this_type& b) const {
		return hand_selections == b.hand_selections;  
	}

	template <class E>
	auto make_viable_setup(const E& character_entity) const;

	template <class C>
	bool is_akimbo(const C& cosm) const;

	auto& flip() {
		std::swap(hand_selections[0], hand_selections[1]);
		return *this;
	}

	template <class E>
	bool same_as_in(const E& character_entity) const {
		return *this == from_current(character_entity);
	}

	friend std::ostream& operator<<(std::ostream& o, const this_type& s) {
		return pretty_print(o, s.hand_selections);
	}

	template <class C, class F>
	decltype(auto) on_more_recent_item(C& cosm, F&& callback) const;
};

using wielding_setup = basic_wielding_setup<entity_id>;
using signi_wielding_setup = basic_wielding_setup<signi_entity_id>;
