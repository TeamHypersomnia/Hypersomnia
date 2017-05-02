#pragma once
#include "game/enums/sentience_meter_type.h"

struct sentience_meter_bar;

struct sentience_meter_bar_in_character_gui {
public:
	typedef sentience_meter_bar dereferenced_type;

	sentience_meter_type type = sentience_meter_type::COUNT;

	bool operator==(const sentience_meter_bar_in_character_gui b) const {
		return type == b.type;
	}

	template <class C>
	bool alive(const C context) const {
		return static_cast<int>(type) < static_cast<int>(sentience_meter_type::COUNT);
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().sentience_meter_bars.at(static_cast<size_t>(type));
	}
};


namespace std {
	template <>
	struct hash<sentience_meter_bar_in_character_gui> {
		size_t operator()(const sentience_meter_bar_in_character_gui& k) const {
			return hash<sentience_meter_type>()(k.type);
		}
	};
}