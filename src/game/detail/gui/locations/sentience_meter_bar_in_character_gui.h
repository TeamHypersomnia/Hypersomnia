#pragma once
#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"

struct sentience_meter_bar;

struct sentience_meter_bar_in_character_gui {
public:
	typedef sentience_meter_bar dereferenced_type;

	unsigned vertical_index = 0xdeadbeef;

	bool operator==(const sentience_meter_bar_in_character_gui b) const {
		return vertical_index == b.vertical_index;
	}

	template <class C>
	bool alive(const C context) const {
		return vertical_index < num_types_in_list_v<spell_instance_tuple> + num_types_in_list_v<perk_instance_tuple>;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().sentience_meter_bars.at(static_cast<size_t>(vertical_index));
	}
};


namespace std {
	template <>
	struct hash<sentience_meter_bar_in_character_gui> {
		size_t operator()(const sentience_meter_bar_in_character_gui& k) const {
			return hash<decltype(k.vertical_index)>()(k.vertical_index);
		}
	};
}