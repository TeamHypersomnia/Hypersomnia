#pragma once
#include "augs/templates/always_false.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/entity_flag.h"

#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/crosshair_component.h"

#include "game/transcendental/specific_entity_handle_declaration.h"

template <class A, class = void>
struct has_specific_entity_type : std::false_type {};

template <class A>
struct has_specific_entity_type<A, decltype(typename A::used_entity_type(), void())> 
	: std::true_type
{};

template <class A>
constexpr bool has_specific_entity_type_v = has_specific_entity_type<A>::value;

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
		return self.get_meta().guid;
	}

	auto get_raw_flavour_id() const {
		const auto self = *static_cast<const E*>(this);
		return self.get_meta().flavour_id;
	}

	auto get_flavour_id() const {
		const auto self = *static_cast<const E*>(this);
		
		entity_flavour_id id;

		id.raw = get_raw_flavour_id();
		id.type_id = self.get_type_id();

		return id;
	}

	auto& get_flavour() const {
		if constexpr(has_specific_entity_type_v<E>) {
			const auto self = *static_cast<const E*>(this);
			auto& cosm = self.get_cosmos();
			return cosm.template get_flavour<entity_type_of<E>>(get_raw_flavour_id());
		}
		else {
			static_assert(always_false_v<E>, "You can't get a flavour out of a non-specific handle.");
			return *this;
		}
	}

	const auto& get_name() const {
		const auto self = *static_cast<const E*>(this);

		auto& cosm = self.get_cosmos();

		if constexpr(has_specific_entity_type_v<E>) {
			return get_flavour().name;
		}
		else {
			return cosm.on_flavour(get_flavour_id(), [](const auto& f) -> const auto& { return f.name; });
		}
	}

	const auto& get_description() const {
		const auto self = *static_cast<const E*>(this);

		auto& cosm = self.get_cosmos();

		if constexpr(has_specific_entity_type_v<E>) {
			return get_flavour().description;
		}
		else {
			return cosm.on_flavour(get_flavour_id(), [](const auto& f) -> const auto& { return f.description; });
		}
	}

	bool sentient_and_unconscious() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sentience = self.template find<components::sentience>()) {
			return !sentience->is_conscious();
		}

		return false;
	}
};
