#pragma once
#include <variant>
#include <optional>
#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"

template <class S, class C, class I>
std::optional<transformr> find_transform_impl(S& self, C& cosm, I& interp);

struct absolute_or_local {
	entity_id target;
	transformr offset;
	bool face_velocity = false;
	bool chase_velocity = false;

	template <class C, class I>
	auto find_transform(C& cosm, I& interp) {
		return find_transform_impl(*this, cosm, interp);
	}

	template <class C, class I>
	auto find_transform(C& cosm, I& interp) const {
		return find_transform_impl(*this, cosm, interp);
	}

	bool operator==(const absolute_or_local& b) const = default;

	auto hash() const {
		return augs::hash_multiple(target, offset, face_velocity, chase_velocity);
	}
};

template <class C, class... I>
std::optional<transformr> find_transform(const absolute_or_local& l, C& cosm, I&&... interp) {
	return l.find_transform(cosm, std::forward<I>(interp)...);
}	

inline auto get_chased(const absolute_or_local& l) {
	return l.target;
}

namespace std {
	template <>
	struct hash<absolute_or_local> {
		std::size_t operator()(const absolute_or_local t) const {
			return t.hash();
		}
	};
}
