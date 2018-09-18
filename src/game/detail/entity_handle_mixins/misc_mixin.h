#pragma once
#include "augs/templates/identity_templates.h"

#include "game/cosmos/entity_id.h"

#include "game/enums/entity_flag.h"
#include "game/enums/faction_type.h"

#include "game/components/flags_component.h"
#include "game/components/sentience_component.h"
#include "game/components/crosshair_component.h"

#include "game/cosmos/specific_entity_handle_declaration.h"
#include "game/detail/sentience_shake.h"
#include "augs/math/physics_structs.h"
#include "game/detail/economy/money_type.h"

void unset_input_flags_of_orphaned_entity(const entity_handle);

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

	void flip_horizontally() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sprite = self.template find<components::sprite>()) {
			auto& f = sprite->flip.horizontally;
			f = !f;
		}
	}

	void flip_vertically() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sprite = self.template find<components::sprite>()) {
			auto& f = sprite->flip.vertically;
			f = !f;
		}
	}

	void do_flip(const flip_flags f) const {
		const auto self = *static_cast<const E*>(this);

		if (f.horizontally) {
			self.flip_horizontally();
		}
		if (f.vertically) {
			self.flip_vertically();
		}
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
			considered_base_offset.rotate(recoil.rotation);

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

	auto when_born() const {
		const auto self = *static_cast<const E*>(this);
		return self.get_meta().when_born;
	}

	auto get_flavour_id() const {
		const auto self = *static_cast<const E*>(this);

		if constexpr(E::is_specific) {
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
		if constexpr(E::is_specific) {
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

		if (const auto name = mapped_or_nullptr(cosm.get_solvable().significant.specific_names, self.get_id())) {
			return *name;
		}

		if constexpr(E::is_specific) {
			return get_flavour().template get<invariants::text_details>().name;
		}
		else {
			return cosm.on_flavour(get_flavour_id(), [](const auto& f) -> const auto& { return f.get_name(); });
		}
	}

	const auto& get_description() const {
		const auto self = *static_cast<const E*>(this);

		auto& cosm = self.get_cosmos();

		if constexpr(E::is_specific) {
			return get_flavour().template get<invariants::text_details>().description;
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

	faction_type get_official_faction() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto attitude = self.template find<components::attitude>()) {
			return attitude->official_faction;
		}

		if (const auto sender = self.template find<components::sender>()) {
			return sender->faction_of_sender;
		}

		if (const auto marker = self.template find<invariants::box_marker>()) {
			return marker->meta.associated_faction;
		}

		if (const auto marker = self.template find<invariants::point_marker>()) {
			return marker->meta.associated_faction;
		}

		return faction_type::SPECTATOR;
	}

	bool is_like_planted_bomb() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto fuse = self.template find<components::hand_fuse>()) {
			if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
				return fuse->armed() && fuse_def->is_like_plantable_bomb();
			}
		}

		return false;
	}

	bool is_like_thrown_explosive() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto fuse = self.template find<components::hand_fuse>()) {
			if (const auto fuse_def = self.template find<invariants::hand_fuse>()) {
				return fuse->armed() && self.get_owning_transfer_capability().dead();
			}
		}

		return false;
	}

	bool is_frozen() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto movement = self.template find<components::movement>()) {
			return movement->frozen;
		}

		return false;
	}

	void set_frozen(const bool flag) const {
		const auto self = *static_cast<const E*>(this);

		if (const auto movement = self.template find<components::movement>()) {
			movement->frozen = flag;

			if (flag) {
				unset_input_flags_of_orphaned_entity(self);
			}
		}
	}

	std::optional<money_type> find_price() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto item = self.template find<invariants::item>()) {
			return item->standard_price;
		}

		return std::nullopt;
	}

	assets::image_id get_image_id() const {
		const auto self = *static_cast<const E*>(this);

		if (const auto sprite = self.template find<invariants::sprite>()) {
			return sprite->image_id;
		}

		return {};
	}
};
