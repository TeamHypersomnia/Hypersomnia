#pragma once
#include "augs/templates/identity_templates.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/entity_flag.h"

#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/crosshair_component.h"

#include "game/transcendental/specific_entity_handle_declaration.h"
#include "game/detail/sentience_shake.h"
#include "augs/math/physics_structs.h"

template <class A, class = void>
struct has_specific_entity_type : std::false_type {};

template <class A>
struct has_specific_entity_type<A, decltype(typename A::used_entity_type(), void())> 
	: std::true_type
{};

template <class A>
constexpr bool has_specific_entity_type_v = has_specific_entity_type<A>::value;

struct simple_body;

template <class E>
class misc_mixin {
public:

	std::optional<flip_flags> calculate_flip_flags() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sprite = self.template find<components::sprite>()) {
			return sprite->flip;	
		}

		return std::nullopt;
	}

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

	auto apply_crosshair_recoil(const impulse_input impulse) const {
		const auto self = *static_cast<const E*>(this);

		if (const auto crosshair = self.find_crosshair()) {
			crosshair->recoil.apply(impulse * self.find_crosshair_def()->recoil_impulse_mult);
		}
	}

	auto calc_crosshair_displacement(
		const bool snap_epsilon_base_offset = false
	) const {
		const auto self = *static_cast<const E*>(this);

		if (const auto crosshair = self.find_crosshair()) {
			auto considered_base_offset = crosshair->base_offset;
			const auto& recoil = crosshair->recoil;

			if (snap_epsilon_base_offset && considered_base_offset.is_epsilon(4)) {
				considered_base_offset.set(4, 0);
			}

			considered_base_offset += recoil.position;
			considered_base_offset.rotate(recoil.rotation, vec2());

			return considered_base_offset;
		}

		return vec2(0, 0);
	}

	void apply_shake(const sentience_shake shake) const {
		const auto self = *static_cast<const E*>(this);

		if (const auto s = self.template find<components::sentience>()) {
			shake.apply(self.get_cosmos().get_timestamp(), *s);
		}
	}

	template <class I>
	auto get_world_crosshair_transform(I& interp) const {
		const auto self = *static_cast<const E*>(this);
		return self.get_viewing_transform(interp) + self.calc_crosshair_displacement();
	};

	auto get_world_crosshair_transform() const {
		const auto self = *static_cast<const E*>(this);
		return self.get_logic_transform() + self.calc_crosshair_displacement();
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

		if constexpr(has_specific_entity_type_v<E>) {
			return typed_entity_flavour_id<entity_type_of<E>>(self.get_raw_flavour_id());
		}
		else {
			entity_flavour_id id;

			id.raw = get_raw_flavour_id();
			id.type_id = self.get_type_id();

			return id;
		}
	}

	auto& get_flavour() const {
		if constexpr(has_specific_entity_type_v<E>) {
			const auto self = *static_cast<const E*>(this);
			auto& cosm = self.get_cosmos();
			return cosm.template get_flavour<entity_type_of<E>>(get_flavour_id());
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
			return get_flavour().template get<invariants::name>().name;
		}
		else {
			return cosm.on_flavour(get_flavour_id(), [](const auto& f) -> const auto& { return f.get_name(); });
		}
	}

	const auto& get_description() const {
		const auto self = *static_cast<const E*>(this);

		auto& cosm = self.get_cosmos();

		if constexpr(has_specific_entity_type_v<E>) {
			return get_flavour().template get<invariants::name>().description;
		}
		else {
			return cosm.on_flavour(get_flavour_id(), [](const auto& f) -> const auto& { return f.get_description(); });
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
