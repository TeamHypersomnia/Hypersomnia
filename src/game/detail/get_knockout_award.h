#pragma once
#include "game/components/explosive_component.h"
#include "game/components/gun_component.h"
#include "game/detail/adversarial_meta.h"

template <class E>
const adversarial_meta* find_adversarial_meta(const E& from) {
	LOG(get_type_name<E>());
	if constexpr(E::template has<invariants::gun>()) {
		return std::addressof(from.template get<invariants::gun>().adversarial);
	}
	else if constexpr(E::template has<invariants::explosive>()) {
		return std::addressof(from.template get<invariants::explosive>().adversarial);
	}

	return nullptr;
}

template <class F>
auto get_knockout_award(const F& flavour) {
	if (const auto* adversarial = find_adversarial_meta(flavour)) {
		LOG_NVPS(adversarial->knockout_award);
		return adversarial->knockout_award;
	}
	LOG("ZERO");

	return static_cast<money_type>(0);
}
