#pragma once
#include "game/detail/damage_origin.h"

inline bool damage_cause::is_humiliating(const cosmos& cosm) const {
	if (flavour.is_set()) {
		return cosm.on_flavour(
			flavour,
			[&](const auto& typed_flavour) {
				return typed_flavour.template has<invariants::melee>();
			}
		);
	}

	return false;
}

template <class E>
void damage_origin::copy_sender_from(const E& causing_handle) {
	if (const auto s = causing_handle.template find<components::sender>()) {
		if (s->is_set()) {
			sender = *s;
		}
		else {
			sender.set(causing_handle);
		}
	}
	else {
		sender.set(causing_handle);
	}
}

template <class E>
auto damage_origin::get_guilty_of_damaging(const E& victim_handle) const {
	const auto faction = victim_handle.get_official_faction();
	auto& cosm = victim_handle.get_cosmos();

	auto is_delayed_enough_explosive = [&](const auto flavour_id) {
		if (!flavour_id.is_set()) {
			return false;
		}

		return flavour_id.dispatch([&](const auto& typed_id) {
			if (const auto flavour = cosm.find_flavour(typed_id)) {
				if (const auto hand_fuse = flavour->template find<invariants::hand_fuse>()) {
					return hand_fuse->is_like_plantable_bomb();
				}
			}

			return false;
		});
	};

	if (sender.faction_of_sender == faction) {
		/* 
			Case: direct cause is an explosion body of a bomb planted by the same faction.
			The sender is the bomb and the sender faction is propagated from the sender of the bomb.
		*/

		if (is_delayed_enough_explosive(sender.direct_sender_flavour)) {
			return victim_handle;
		}

		/* 
			Case: direct cause is a bomb planted by the same faction.
			The sender is the player and its faction is set.
		*/

		if (is_delayed_enough_explosive(cause.flavour)) {
			return victim_handle;
		}
	}

	return cosm[sender.capability_of_sender];
}


template <class C, class F>
auto damage_origin::on_tool_used(C& cosm, F callback) const {
	if (cause.spell.is_set()) {
		return cause.spell.dispatch([&](auto s) {
			using S = decltype(s);
			return callback(std::get<S>(cosm.get_common_significant().spells));
		});
	}

	using R = decltype(callback(haste()));
	using opt_R = std::optional<R>;

	auto from_flavour = [&](const auto flavour_id) -> opt_R {
		if (!flavour_id.is_set()) {
			return std::nullopt;
		}

		return flavour_id.dispatch([&](const auto& typed_flavour_id) -> opt_R {
			if (const auto flavour = cosm.find_flavour(typed_flavour_id)) {
				if constexpr(remove_cref<decltype(*flavour)>::template has<invariants::sentience>()) {
					return std::nullopt;
				}
				else {
					return callback(typed_flavour_id);
				}
			}

			return std::nullopt;
		});
	};

	if (const auto result = from_flavour(sender.direct_sender_flavour)) {
		return *result;
	}

	if (const auto result = from_flavour(cause.flavour)) {
		return *result;
	}

	return callback(std::nullopt);
}
