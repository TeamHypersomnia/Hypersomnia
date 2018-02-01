#pragma once
#include "game/transcendental/entity_id.h"

#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/flavour_component.h"

template <class E>
class misc_mixin {
public:
	bool get_flag(const entity_flag f) const {
		const auto self = *static_cast<const E*>(this);
		ensure(self.alive());
		return self.template get<invariants::flags>().values.test(f);
	}

	entity_guid get_guid() const {
		const auto self = *static_cast<const E*>(this);
		return self.get_cosmos().get_solvable().get_guid(self.get_id());
	}

	const auto& get_flavour() const {
		const auto self = *static_cast<const E*>(this);
		return self.template get<components::flavour>().get_flavour();
	}

	auto get_flavour_id() const {
		const auto self = *static_cast<const E*>(this);
		return self.template get<components::flavour>().get_flavour_id();
	}

	const auto& get_name() const {
		const auto self = *static_cast<const E*>(this);
		return self.template get<components::flavour>().get_name();
	}

	bool sentient_and_unconscious() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sentience = self.template find<components::sentience>()) {
			return !sentience->is_conscious();
		}

		return false;
	}
};
