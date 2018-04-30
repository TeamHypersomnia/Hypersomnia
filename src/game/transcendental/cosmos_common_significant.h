#pragma once
#include <tuple>

#include "augs/math/si_scaling.h"

#include "augs/templates/type_in_list_id.h"
#include "augs/templates/identity_templates.h"
#include "augs/templates/get_by_dynamic_id.h"

#include "game/common_state/visibility_settings.h"
#include "game/common_state/pathfinding_settings.h"
#include "game/common_state/common_assets.h"
#include "game/common_state/entity_flavours.h"

#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"
#include "game/detail/all_sentience_meters.h"

#include "game/assets/all_logical_assets.h"

using meter_tuple = meter_list_t<std::tuple>;
using spell_tuple = spell_list_t<std::tuple>;
using perk_tuple = perk_list_t<std::tuple>;

using spell_meta_id = type_in_list_id<spell_tuple>;
using perk_meta_id = type_in_list_id<meter_tuple>;

struct cosmos_common_significant {
	// GEN INTROSPECTOR struct cosmos_common_significant
	all_logical_assets logical_assets;

	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

	all_entity_flavours flavours;
	common_assets assets;

	meter_tuple meters;
	spell_tuple spells;
	perk_tuple perks;

	rgba ambient_light_color = { 25, 51, 51, 255 };
	// END GEN INTROSPECTOR

	template <class entity_type>
	auto& get_flavours() {
		return std::get<make_entity_flavours<entity_type>>(flavours);	
	}

	template <class entity_type>
	const auto& get_flavours() const {
		return std::get<make_entity_flavours<entity_type>>(flavours);	
	}

private:
	template <class C, class... Types, class F>
	static decltype(auto) on_flavour_impl(
		C& self,
		const constrained_entity_flavour_id<Types...> flavour_id,
		F callback	
	) {
		using candidate_types = typename decltype(flavour_id)::matching_types; 

		return conditional_get_by_dynamic_id<candidate_types>(
			all_entity_types(),
			flavour_id.type_id,
			[&](auto t) -> decltype(auto) {
				using E = decltype(t);
				return callback(self.template get_flavours<E>().get_flavour(typed_entity_flavour_id<E>(flavour_id.raw)));
			}
		);
	}

	template <class C, class E, class F>
	static decltype(auto) on_flavour_impl(
		C& self,
		const typed_entity_flavour_id<E> flavour_id,
		F callback	
	) {
		using candidate_types = typename decltype(flavour_id)::matching_types; 
		return callback(self.template get_flavours<E>().get_flavour(flavour_id));
	}

public:
	template <class T, class F>
	decltype(auto) on_flavour(const T flavour_id, F&& callback) {
		return on_flavour_impl(*this, flavour_id, std::forward<F>(callback));
	}

	template <class T, class F>
	decltype(auto) on_flavour(const T flavour_id, F&& callback) const {
		return on_flavour_impl(*this, flavour_id, std::forward<F>(callback));
	}
};

namespace std {
	template <class T>
	auto& get(const cosmos_common_significant& s) {
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
	const auto& get(const cosmos_common_significant& s) {
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