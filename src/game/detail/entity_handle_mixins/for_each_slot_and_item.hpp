#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/inventory/direct_attachment_offset.h"
#include "augs/templates/continue_or_callback_result.h"
#include "augs/templates/traits/is_nullopt.h"

template <class E>
template <class S, class I>
callback_result inventory_mixin<E>::for_each_contained_slot_and_item_recursive(
	S&& slot_callback, 
	I&& item_callback,
	const optional_slot_flags& filter
) const {
	const auto this_container = *static_cast<const E*>(this);
	auto& cosm = this_container.get_cosmos();

	return this_container.template dispatch_on_having_all_ret<invariants::container>([&](const auto& typed_container) {
		if constexpr(is_nullopt_v<decltype(typed_container)>) {
			return callback_result::CONTINUE;
		}
		else {
			auto& container = typed_container.template get<invariants::container>();

			for (auto&& s : container.slots) {
				if (filter && !filter->test(s.first)) {
					continue;
				}

				const auto this_slot_id = inventory_slot_id(s.first, typed_container.get_id());
				const auto slot_callback_result = continue_or_recursive_callback_result(std::forward<S>(slot_callback), cosm[this_slot_id]);

				if (slot_callback_result == recursive_callback_result::ABORT) {
					return callback_result::ABORT;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
					continue;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_AND_RECURSE) {
					const auto items = typed_container[s.first].get_items_inside();
					
					for (const auto& id : items) {
						const auto result = cosm[id].template dispatch_on_having_all_ret<components::item>(
							[&](const auto& child_item_handle) {
								if constexpr(!is_nullopt_v<decltype(child_item_handle)>) {
									const auto r = continue_or_recursive_callback_result(std::forward<I>(item_callback), child_item_handle);

									if (r == recursive_callback_result::CONTINUE_AND_RECURSE) {
										const auto next_r = child_item_handle.for_each_contained_slot_and_item_recursive(std::forward<S>(slot_callback), std::forward<I>(item_callback), filter);

										if (callback_result::ABORT == next_r) {
											return recursive_callback_result::ABORT;
										}
									}

									return r;
								}
								else {
									return recursive_callback_result::ABORT;
								}
							}
						);

						if (result == recursive_callback_result::ABORT) {
							return callback_result::ABORT;
						}
						else if (result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
							continue;
						}
					}
				}
			}

			return callback_result::CONTINUE;
		}
	});
}

template <class E>
template <class A, class B, class G>
void inventory_mixin<E>::recurse_character_attachments(
	A attachment_callback,
	B should_recurse,
	G get_offsets_by_torso,
	const attachment_offset_settings& settings,
	const bool flip_hands_order
) const {
	const auto character = *static_cast<const E*>(this);

	auto get_offset_for = [&](
		const auto for_attachment,
		const auto for_type
	) {
		return direct_attachment_offset(
			character,
			for_attachment,
			get_offsets_by_torso,
			settings,
			for_type
		);
	};

	auto do_callback_for = [&](const slot_function type) {
		const auto slot = character[type];

		if (!should_recurse(slot)) {
			return;
		}

		slot.dispatch_on_item_inside(
			[&](const auto& typed_inside) {
				const auto total_offset = get_offset_for(typed_inside, type);
				attachment_callback(typed_inside, total_offset);
			}
		);
	};

	auto recurse_attachments_in = [&](const slot_function type) {
		const auto slot = character[type];

		if (!should_recurse(slot)) {
			return;
		}

		slot.dispatch_on_item_inside(
			[&](const auto& typed_inside) {
				const auto total_offset = get_offset_for(typed_inside, type);

				typed_inside.with_each_attachment_recursive(
					attachment_callback,
					settings,
					total_offset
				);
			}
		);
	};

	auto hand_1 = slot_function::PRIMARY_HAND;
	auto hand_2 = slot_function::SECONDARY_HAND;

	if (flip_hands_order) {
		std::swap(hand_1, hand_2);
	}

	recurse_attachments_in(hand_1);
	recurse_attachments_in(hand_2);

	do_callback_for(slot_function::BELT);
	do_callback_for(slot_function::BACK);
	do_callback_for(slot_function::OVER_BACK);
	do_callback_for(slot_function::SHOULDER);
}

template <class E>
template <class A>
void inventory_mixin<E>::with_each_attachment_recursive(
	A attachment_callback,
	const attachment_offset_settings& settings,
	const attachment_offset initial_offset
) const {
	const auto item_with_attachments = *static_cast<const E*>(this);

	auto get_offset_for = [&](
		const auto for_attachment,
		const auto for_type
	) {
		return initial_offset * direct_attachment_offset(
			item_with_attachments,
			for_attachment,
			[]() { return torso_offsets(); },
			settings,
			for_type
		);
	};

	if constexpr(E::template has<invariants::gun>()) {
		auto do_callback_for = [&](const slot_function type, const bool only_under) {
			const auto slot = item_with_attachments[type];

			if (slot.dead()) {
				return;
			}

			const bool should_process = slot->makes_physical_connection();

			if (!should_process) {
				return;
			}

			if (only_under != slot->draw_under_container) {
				return;
			}

			slot.dispatch_on_item_inside(
				[&](const auto& typed_inside) {
					const auto total_offset = get_offset_for(typed_inside, type);
					attachment_callback(typed_inside, total_offset);
				}
			);
		};

		const auto gun_attachments = std::array<slot_function, 5> {
			slot_function::GUN_CHAMBER_MAGAZINE,
			slot_function::GUN_CHAMBER,
			slot_function::GUN_DETACHABLE_MAGAZINE,
			slot_function::GUN_RAIL,
			slot_function::GUN_MUZZLE
		};

		for (const auto& s : gun_attachments) {
			do_callback_for(s, true);
		}

		attachment_callback(item_with_attachments, initial_offset);

		for (const auto& s : gun_attachments) {
			do_callback_for(s, false);
		}
	}
	else {
		attachment_callback(item_with_attachments, initial_offset);
	}
}

template <class E>
template <class I>
void inventory_mixin<E>::for_each_contained_item_recursive(I&& item_callback, const optional_slot_flags& filter) const {
	for_each_contained_slot_and_item_recursive(
		[](auto) {}, 
		std::forward<I>(item_callback),
		filter
	);
}

template <class E>
template <class S>
void inventory_mixin<E>::for_each_contained_slot_recursive(S&& slot_callback, const optional_slot_flags& filter) const {
	for_each_contained_slot_and_item_recursive(
		std::forward<S>(slot_callback), 
		[](auto...) {},
		filter
	);
}

template <class E>
ltrb inventory_mixin<E>::calc_aabb_with_attachments() const {
	ltrb result;

	with_each_attachment_recursive(
		[&result](
			const auto attachment_entity,
			const auto attachment_offset
		) {
			result.contain(attachment_entity.get_aabb(attachment_offset.offset));
		},
		attachment_offset_settings::for_logic()
	);

	return result;
}
