#pragma once
#include <variant>
#include <optional>
#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"

struct orbital_chasing {
	entity_id target;
	components::transform offset;

	template <class C, class I>
	std::optional<components::transform> find_transform(C& cosm, I& interp) const {
		if (const auto target_transform  = cosm[target].find_logic_transform()) {
			auto result = *target_transform + offset;
			result.pos.rotate(target_transform->rotation, target_transform->pos);

			return result;
		}

		return std::nullopt;
	}

	bool operator==(const orbital_chasing b) const {
		return target == b.target && offset == b.offset;
	}
};

using absolute_or_local = std::variant<components::transform, orbital_chasing>;

template <class C, class I>
std::optional<components::transform> find_transform(const absolute_or_local& l, C& cosm, I& interp) {
	if (auto chasing = std::get_if<orbital_chasing>(&l)) {
		return chasing->find_transform(cosm, interp);
	}

	return std::get<components::transform>(l);
}	

namespace std {
	template <class H>
	struct hash;

	template <>
	struct hash<orbital_chasing> {
		std::size_t operator()(const orbital_chasing t) const {
			return augs::simple_two_hash(t.target, t.offset);
		}
	};
}
