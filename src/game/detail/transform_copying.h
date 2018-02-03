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
		if (const auto target_transform  = cosm[target].find_viewing_transform(interp)) {
			return *result * offset;
		}

		return std::nullopt;
	}

	template <class C>
	std::optional<components::transform> find_transform(C& cosm) const {
		if (const auto target_transform  = cosm[target].find_logic_transform()) {
			return *result * offset;
		}

		return std::nullopt;
	}

	bool operator==(const orbital_chasing b) const {
		return target == b.target && offset == b.offset;
	}
};

using absolute_or_local = std::variant<components::transform, orbital_chasing>;

template <class C, class... I>
std::optional<components::transform> find_transform(const absolute_or_local& l, C& cosm, I&&... interp) {
	if (auto chasing = std::get_if<orbital_chasing>(&l)) {
		return chasing->find_transform(cosm, std::forward<I>(interp)...);
	}

	return std::get<components::transform>(l);
}	

inline auto get_chased(const absolute_or_local& l) {
	if (auto chasing = std::get_if<orbital_chasing>(&l)) {
		return chasing->target;
	}

	return {};
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
