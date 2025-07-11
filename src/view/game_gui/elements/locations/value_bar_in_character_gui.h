#pragma once
#include <cstddef>
#include "augs/templates/type_list.h"

#include "game/detail/all_sentience_meters.h"
#include "game/detail/perks/all_perks.h"

struct value_bar;

constexpr std::size_t value_bar_count =
	num_types_in_list_v<meter_instance_tuple> +
	num_types_in_list_v<perk_instance_tuple>
;

struct value_bar_in_character_gui {
public:
	using dereferenced_type = value_bar;

	unsigned vertical_index = 0xdeadbeef;

	bool operator==(const value_bar_in_character_gui b) const {
		return vertical_index == b.vertical_index;
	}

	template <class C>
	bool alive(const C) const {
		return vertical_index < value_bar_count;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().value_bars.at(static_cast<size_t>(vertical_index));
	}
};


namespace std {
	template <>
	struct hash<value_bar_in_character_gui> {
		size_t operator()(const value_bar_in_character_gui& k) const {
			return hash<decltype(k.vertical_index)>()(k.vertical_index);
		}
	};
}