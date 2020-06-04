#pragma once
#include "game/components/explosive_component.h"
#include "game/components/gun_component.h"
#include "game/detail/adversarial_meta.h"

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
