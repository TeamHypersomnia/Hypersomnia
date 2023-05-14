#pragma once
#include "game/detail/flavour_scripts.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/modes/detail/flavour_getters.h"
#include "game/modes/detail/spell_getters.h"
#include "game/enums/filters.h"

template <class E>
int num_carryable_pieces(
	const E& in_subject,
	const candidate_holster_types& opts,
	const item_flavour_id& item
) {
	int total_fitting = 0;

	const auto& cosm = in_subject.get_cosmos();

	cosm.on_flavour(
		item,
		[&](const auto& typed_flavour) {
			const auto& item_def = typed_flavour.template get<invariants::item>();

			const auto piece_occupied_space = calc_space_occupied_of_purchased(cosm, item);

			auto check_slot = [&](const auto& slot) {
				if (slot.dead()) {
					return;
				}

				if (slot->is_category_compatible_with(item, item_def.categories_for_slot_compatibility)) {
					if (slot->always_allow_exactly_one_item) {
						if (slot.is_empty_slot()) {
							++total_fitting;
						}
					}
					else {
						total_fitting += slot.calc_real_space_available() / piece_occupied_space;
					}
				}
			};

			in_subject.for_each_candidate_slot(
				opts,
				check_slot
			);
		}
	);

	return total_fitting;
}

template <class E>
std::optional<money_type> find_price_of(const cosmos& cosm, const E& object) {
	if constexpr(std::is_same_v<E, item_flavour_id>) {
		if (!is_alive(cosm, object)) {
			return static_cast<money_type>(0);
		}

		return cosm.on_flavour(object, [&](const auto& typed_flavour) {
			return typed_flavour.find_price();
		});
	}
	else {
		return on_spell(cosm, object, [&](const auto& spell_data) {
			return spell_data.common.standard_price;
		});
	}
}

inline auto get_buy_slot_opts() {
	return candidate_holster_types {
		candidate_holster_type::WEARABLES,
		candidate_holster_type::HANDS,
		candidate_holster_type::CONTAINERS
	};
}

inline bool is_magazine_like(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		return typed_flavour.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE);
	});
}

inline bool is_armor_like(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		return typed_flavour.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::TORSO_ARMOR);
	});
}

template <class H>
inline bool is_ammo_piece_like(const H& handle) {
	const bool is_charge_like = handle.template has<invariants::cartridge>();

	return is_charge_like || ::is_magazine_like(handle.get_cosmos(), handle.get_flavour_id());
}

inline bool is_shotgun_like(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		if (const auto gun = typed_flavour.template find<invariants::gun>()) {
			return gun->shot_cooldown_ms >= 200.f && repro::fabs(gun->muzzle_velocity.first - gun->muzzle_velocity.second) >= 1000.f;
		}

		return false;
	});

	return false;
}

template <class E, class T>
bool factions_compatible(
	const E& subject, 
	const T& object
) {
	const auto& cosm = subject.get_cosmos();

	auto compatible = [&](const auto& f) {
		const auto subject_faction = subject.get_official_faction();
		return f == faction_type::SPECTATOR || subject_faction == f;
	};

	if constexpr(std::is_same_v<T, spell_id>) {
		return on_spell(cosm, object, [&](const auto& spell) {
			return compatible(spell.common.specific_to);
		});
	}
	else {
		return cosm.on_flavour(object, [&](const auto& typed_flavour) {
			return compatible(typed_flavour.template get<invariants::item>().specific_to);
		});
	}
}

template <class T>
bool is_backpack_like(const T& handle) {
	if (const auto item = handle.template find<invariants::item>()) {
		return item->categories_for_slot_compatibility.test(item_category::BACK_WEARABLE);
	}

	return false;
}

enum class once_owning_category {
	NONE,

	DEFUSER_LIKE,
	ARMOR_LIKE,
	BACKPACK_LIKE
};

inline once_owning_category calc_once_owning_category(const cosmos& cosm, const item_flavour_id& id) {
	if (::is_armor_like(cosm, id)) {
		return once_owning_category::ARMOR_LIKE;
	}

	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		if (const auto tool = typed_flavour.template find<invariants::tool>()) {
			if (tool->spell_cost_amortization > 0.f) {
				return once_owning_category::NONE;
			}

			if (tool->defusing_speed_mult > 1.f) {
				return once_owning_category::DEFUSER_LIKE;
			}
		}

		if (::is_backpack_like(typed_flavour)) {
			return once_owning_category::BACKPACK_LIKE;
		}

		return once_owning_category::NONE;
	});
}

template <class E>
void play_learnt_spell_effect(
	const E& subject,
	const spell_id& id,
	const logic_step step
) {
	const auto& cosm = step.get_cosmos();
	const auto& assets = cosm.get_common_assets();

	{
		const auto& effect = assets.standard_learnt_spell_sound;
		effect.start(step, sound_effect_start_input::at_listener(subject), predictable_only_by(subject));
	}

	{
		const auto col = on_spell(cosm, id, [&](const auto& spell) {
			return spell.common.associated_color;
		});

		auto effect = assets.standard_learnt_spell_particles;
		effect.modifier.color = col;
		effect.start(step, particle_effect_start_input::orbit_local(subject, {}).set_homing(subject), predictable_only_by(subject));
	}
}

template <class E>
auto calc_purchasable_ammo_piece(const E& handle) {
	item_flavour_id result;

	if (const auto mag_slot = handle[slot_function::GUN_DETACHABLE_MAGAZINE]) {
		result = mag_slot->only_allow_flavour;
	}

	if (const auto chamber_mag_slot = handle[slot_function::GUN_CHAMBER_MAGAZINE]) {
		result = chamber_mag_slot->only_allow_flavour;
	}

	return result;
}

template <class E>
auto calc_default_charge_flavour(const E& handle) {
	if (const auto chamber_slot = handle[slot_function::GUN_CHAMBER]) {
		if (const auto f = chamber_slot->only_allow_flavour; f.is_set()) {
			return f;
		}
	}

	if (const auto chamber_slot = handle[slot_function::GUN_CHAMBER_MAGAZINE]) {
		if (const auto f = chamber_slot->only_allow_flavour; f.is_set()) {
			return f;
		}
	}

	if (const auto mag_slot = handle[slot_function::GUN_DETACHABLE_MAGAZINE]) {
		if (const auto only_mag = mag_slot->only_allow_flavour; only_mag.is_set()) {
			if (const auto f = get_allowed_flavour_of_deposit(handle.get_cosmos(), only_mag); f.is_set()) {
				return f;
			}
		}
	}

	return item_flavour_id();
}

template <class E>
auto calc_gun_bullet_physical_filter(const E& subject_gun) {
	auto filter = filters[predefined_filter_type::FLYING_BULLET];

	if (const auto charge_flavour = ::calc_default_charge_flavour(subject_gun); charge_flavour.is_set()) {
		auto& cosm = subject_gun.get_cosmos();

		cosm.on_flavour(
			charge_flavour,
			[&](const auto& typed_charge_flavour) {
				if (const auto cartridge_def = typed_charge_flavour.template find<invariants::cartridge>()) {
					if (const auto round_flavour = cartridge_def->round_flavour; round_flavour.is_set()) {
						cosm.on_flavour(
							round_flavour,
							[&](const auto& typed_round_flavour) {
								filter = typed_round_flavour.template get<invariants::fixtures>().filter;
							}
						);
					}
				}
			}
		);
	}

	return filter;
}

template <class E>
auto calc_all_ammo_pieces_of(const E& handle) {
	std::vector<item_flavour_id> results;

	auto push_if_set = [&results](const auto f) {
		if (f.is_set()) {
			results.push_back(f);
		}
	};

	if (const auto mag_slot = handle[slot_function::GUN_DETACHABLE_MAGAZINE]) {
		const auto f = mag_slot->only_allow_flavour;

		push_if_set(f);
		push_if_set(get_allowed_flavour_of_deposit(handle.get_cosmos(), f));
	}

	if (const auto chamber_mag_slot = handle[slot_function::GUN_CHAMBER_MAGAZINE]) {
		push_if_set(chamber_mag_slot->only_allow_flavour);
	}

	if (const auto chamber_slot = handle[slot_function::GUN_CHAMBER]) {
		push_if_set(chamber_slot->only_allow_flavour);
	}

	return results;
}
