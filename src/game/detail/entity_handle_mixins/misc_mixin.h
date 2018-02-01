#pragma once
#include "game/transcendental/entity_id.h"

#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/flavour_component.h"
#include "game/components/crosshair_component.h"

template <class E>
class misc_mixin {
public:
	auto* find_crosshair() const {
		/* If it were other entity for some reason */
		const auto self = *static_cast<const E*>(this);
		return self.template find<components::crosshair>();
	};

	auto* find_crosshair_def() const {
		/* If it were other entity for some reason */
		const auto self = *static_cast<const E*>(this);
		return self.template find<invariants::crosshair>();
	};

	auto find_crosshair_recoil() const {
		/* If it were other entity for some reason */
		const auto self = *static_cast<const E*>(this);

		if (const auto c = self.find_crosshair()) {
			return self.get_cosmos()[c->recoil_entity];
		}

		return self.get_cosmos()[entity_id()];
	};

	auto calculate_crosshair_displacement(
		const bool snap_epsilon_base_offset = false
	) const {
		const auto self = *static_cast<const E*>(this);

		const auto recoil_body = self.find_crosshair_recoil();
		const auto recoil_body_transform = recoil_body.get_logic_transform();
		const auto crosshair = self.find_crosshair();

		auto considered_base_offset = crosshair->base_offset;

		if (snap_epsilon_base_offset && considered_base_offset.is_epsilon(4)) {
			considered_base_offset.set(4, 0);
		}

		considered_base_offset += recoil_body_transform.pos;
		considered_base_offset.rotate(recoil_body_transform.rotation, vec2());

		return considered_base_offset;
	}

	template <class I>
	auto get_world_crosshair_transform(I& interp, const bool integerize = false) const {
		const auto self = *static_cast<const E*>(this);
		return self.get_viewing_transform(interp, integerize) + self.calculate_crosshair_displacement();
	};

	auto get_world_crosshair_transform() const {
		const auto self = *static_cast<const E*>(this);
		return self.get_logic_transform() + self.calculate_crosshair_displacement();
	};

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
