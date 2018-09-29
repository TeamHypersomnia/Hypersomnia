#pragma once
#include <array>
#include "augs/string/pretty_print.h"

static constexpr std::size_t hand_count_v = 2;
using hand_selections_array = std::array<entity_id, hand_count_v>;

struct wielding_setup {
	// GEN INTROSPECTOR struct wielding_setup
	hand_selections_array hand_selections;
	// END GEN INTROSPECTOR

	bool operator==(const wielding_setup b) const {
		return hand_selections == b.hand_selections;  
	}

	template <class E>
	auto make_viable_setup(const E& character_entity) const;

	friend std::ostream& operator<<(std::ostream& o, const wielding_setup& s) {
		return pretty_print(o, s.hand_selections);
	}
};
