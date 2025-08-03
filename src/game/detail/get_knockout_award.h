#pragma once
#include "game/components/explosive_component.h"
#include "game/components/gun_component.h"
#include "game/detail/adversarial_meta.h"
#include "game/detail/damage_origin.hpp"

template <class E>
const adversarial_meta* find_adversarial_meta(const E& from) {
	if constexpr(E::template has<invariants::gun>()) {
		return std::addressof(from.template get<invariants::gun>().adversarial);
	}
	else if constexpr(E::template has<invariants::explosive>()) {
		return std::addressof(from.template get<invariants::explosive>().adversarial);
	}
	else if constexpr(E::template has<invariants::melee>()) {
		return std::addressof(from.template get<invariants::melee>().adversarial);
	}

	return nullptr;
}

template <class F>
auto get_knockout_award(const cosmos& cosm, const F& flavour_id) {
	return cosm.on_flavour(flavour_id, [&](const auto& typed_flavour) -> std::optional<money_type> {
		if constexpr(!is_nullopt_v<decltype(typed_flavour)>) {
			if (const auto* adversarial = find_adversarial_meta(typed_flavour)) {
				return adversarial->knockout_award;
			}
		}

		return std::nullopt;
	});
}

inline std::optional<money_type> get_knockout_award(const cosmos& cosm, const damage_origin origin) {
	const auto award = origin.on_tool_used(cosm, [&cosm](const auto& tool) -> std::optional<money_type> {
		if constexpr(is_spell_v<decltype(tool)>) {
			return tool.common.adversarial.knockout_award;
		}
		else if constexpr(is_nullopt_v<decltype(tool)>) {
			return std::nullopt;
		}
		else {
			return ::get_knockout_award(cosm, tool);
		}
	});

	if (award) {
		const auto bonus = money_type(origin.circumstances.headshot ? 500 : 0);

		return *award + bonus;
	}

	return std::nullopt;
}

