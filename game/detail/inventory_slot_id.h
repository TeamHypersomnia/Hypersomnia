#pragma once
#include "game/enums/slot_function.h"
#include "game/transcendental/entity_id.h"

struct inventory_slot_id {
	slot_function type;
	entity_id container_entity;

	inventory_slot_id();
	inventory_slot_id(const slot_function, const entity_id);

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(type),
			CEREAL_NVP(container_entity)
		);
	}

	void unset();

	bool operator<(const inventory_slot_id& b) const;
	bool operator==(const inventory_slot_id& b) const;
	bool operator!=(const inventory_slot_id& b) const;
};

namespace std {
	template <>
	struct hash<inventory_slot_id> {
		std::size_t operator()(const inventory_slot_id& k) const {
			using std::hash;

			return ((hash<entity_id>()(k.container_entity)
				^ (hash<slot_function>()(k.type) << 1)) >> 1);
		}
	};
}