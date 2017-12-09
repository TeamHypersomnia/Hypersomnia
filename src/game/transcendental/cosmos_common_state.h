#pragma once
#include <tuple>

#include "augs/math/si_scaling.h"
#include "augs/templates/type_in_list_id.h"
#include "augs/templates/always_false.h"

#include "game/transcendental/cosmos_meta.h"

#include "game/common_state/visibility_settings.h"
#include "game/common_state/pathfinding_settings.h"
#include "game/common_state/common_assets.h"
#include "game/common_state/entity_name_metas.h"

#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"
#include "game/detail/all_sentience_meters.h"

using meter_tuple = meter_list_t<std::tuple>;
using spell_tuple = spell_list_t<std::tuple>;
using perk_tuple = perk_list_t<std::tuple>;

using spell_meta_id = type_in_list_id<spell_tuple>;
using perk_meta_id = type_in_list_id<meter_tuple>;

struct cosmos_common_state {
	// GEN INTROSPECTOR struct cosmos_common_state
	cosmos_meta meta;

	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

	entity_name_metas name_metas;
	common_assets assets;

	meter_tuple meters;
	spell_tuple spells;
	perk_tuple perks;
	// END GEN INTROSPECTOR
};

namespace std {
	template <class T>
	auto& get(const cosmos_common_state& s) {
		if constexpr(is_one_of_list_v<T, decltype(s.meters)>) {
			return std::get<T>(s.meters);
		}
		else if constexpr(is_one_of_list_v<T, decltype(s.perks)>) {
			return std::get<T>(s.perks);
		}
		else {
			static_assert(always_false_v<T>);
		}
	}

	template <class T>
	const auto& get(const cosmos_common_state& s) {
		if constexpr(is_one_of_list_v<T, decltype(s.meters)>) {
			return std::get<T>(s.meters);
		}
		else if constexpr(is_one_of_list_v<T, decltype(s.perks)>) {
			return std::get<T>(s.perks);
		}
		else {
			static_assert(always_false_v<T>);
		}
	}
}