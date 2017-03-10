#pragma once
#include "game/enums/perk_meter_type.h"

struct perk_meter_bar;

struct perk_meter_bar_in_character_gui {
public:
	typedef perk_meter_bar dereferenced_type;

	perk_meter_type type = perk_meter_type::COUNT;

	bool operator==(const perk_meter_bar_in_character_gui b) const {
		return type == b.type;
	}

	template <class C>
	bool alive(const C context) const {
		return static_cast<int>(type) < static_cast<int>(perk_meter_type::COUNT);
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().perk_meters.at(static_cast<size_t>(type));
	}
};


namespace std {
	template <>
	struct hash<perk_meter_bar_in_character_gui> {
		size_t operator()(const perk_meter_bar_in_character_gui& k) const {
			return hash<perk_meter_type>()(k.type);
		}
	};
}