#pragma once
#include "game/detail/flavour_scripts.h"
#include "game/cosmos/entity_flavour_id.h"

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

template <class F>
decltype(auto) on_spell(const cosmos& cosm, const spell_id& id, F&& callback) {
	return id.dispatch(
		[&](auto s) -> decltype(auto) {
			using S = decltype(s);
			return callback(std::get<S>(cosm.get_common_significant().spells));
		}
	);
};

inline bool is_alive(const cosmos& cosm, const item_flavour_id& t) {
	if (!t.is_set()) {
		return false;
	}

	return t.dispatch(
		[&](const auto& typed_id) {
			return nullptr != cosm.find_flavour(typed_id);
		}
	);
};

inline bool is_alive(const cosmos& cosm, const spell_id& t) {
	(void)cosm;
	return t.is_set();
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

inline item_flavour_id get_allowed_flavour_of_deposit(const cosmos& cosm, const item_flavour_id& mag_id) {
	return cosm.on_flavour(mag_id, [&](const auto& typed_flavour) {
		if (const auto c = typed_flavour.template find<invariants::container>()) {
			if (const auto depo = mapped_or_nullptr(c->slots, slot_function::ITEM_DEPOSIT)) {
				return depo->only_allow_flavour;
			}
		}

		return item_flavour_id();
	});
}

inline bool is_magazine_like(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		return typed_flavour.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE);
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
			return gun->shot_cooldown_ms >= 200.f && std::fabs(gun->muzzle_velocity.first - gun->muzzle_velocity.second) >= 1000.f;
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

inline bool makes_sense_to_only_own_one(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		return typed_flavour.template has<invariants::tool>() || is_backpack_like(typed_flavour);
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
		effect.start(step, sound_effect_start_input::at_entity(subject));
	}

	{
		const auto col = on_spell(cosm, id, [&](const auto& spell) {
			return spell.common.associated_color;
		});

		auto effect = assets.standard_learnt_spell_particles;
		effect.modifier.colorize = col;
		effect.start(step, particle_effect_start_input::orbit_local(subject, {}).set_homing(subject));
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
auto calc_all_ammo_pieces_of(const E& handle) {
	std::vector<item_flavour_id> results;

	if (const auto mag_slot = handle[slot_function::GUN_DETACHABLE_MAGAZINE]) {
		const auto f = mag_slot->only_allow_flavour;

		results.push_back(f);
		results.push_back(get_allowed_flavour_of_deposit(handle.get_cosmos(), f));
	}

	if (const auto chamber_mag_slot = handle[slot_function::GUN_CHAMBER_MAGAZINE]) {
		results.push_back(chamber_mag_slot->only_allow_flavour);
	}

	if (const auto chamber_slot = handle[slot_function::GUN_CHAMBER]) {
		results.push_back(chamber_slot->only_allow_flavour);
	}

	return results;
}
