#pragma once
#include "game/detail/flavour_scripts.h"
#include "game/cosmos/entity_flavour_id.h"

template <class E>
int num_carryable_pieces(
	const E& in_subject,
	const slot_finding_opts& opts,
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
auto get_price_of(const cosmos& cosm, const E& object) {
	if constexpr(std::is_same_v<E, item_flavour_id>) {
		if (!is_alive(cosm, object)) {
			return static_cast<money_type>(0);
		}

		return cosm.on_flavour(object, [&](const auto& typed_flavour) {
			return *typed_flavour.find_price();
		});
	}
	else {
		return on_spell(cosm, object, [&](const auto& spell_data) {
			return spell_data.common.standard_price;
		});
	}
}

inline auto get_buy_slot_opts() {
	return slot_finding_opts {
		slot_finding_opt::CHECK_WEARABLES,
		slot_finding_opt::CHECK_HANDS,
		slot_finding_opt::CHECK_CONTAINERS
	};
}

inline bool is_magazine_like(const cosmos& cosm, const item_flavour_id& id) {
	return cosm.on_flavour(id, [&](const auto& typed_flavour) {
		return typed_flavour.template get<invariants::item>().categories_for_slot_compatibility.test(item_category::MAGAZINE);
	});
}

