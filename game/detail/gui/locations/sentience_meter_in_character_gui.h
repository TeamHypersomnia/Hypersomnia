#pragma once
#include "game/enums/sentience_meter_type.h"

struct sentience_meter;

struct sentience_meter_in_character_gui {
public:
	typedef sentience_meter dereferenced_type;

	sentience_meter_type type;

	bool operator==(const sentience_meter_in_character_gui b) const {
		return true;
	}

	template <class C>
	bool alive(const C context) const {
		return true;
	}

	template <class C>
	decltype(auto) dereference(const C context) const {
		return &context.get_character_gui().sentience_meters.at(static_cast<size_t>(type));
	}
};


namespace std {
	template <>
	struct hash<sentience_meter_in_character_gui> {
		size_t operator()(const sentience_meter_in_character_gui& k) const {
			return hash<sentience_meter_type>()(k.type);
		}
	};
}