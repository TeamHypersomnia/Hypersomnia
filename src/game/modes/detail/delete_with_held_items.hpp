#pragma once
#include "game/messages/will_soon_be_deleted.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"

class arena_mode;

void reverse_perform_deletions(const deletion_queue& deletions, cosmos& cosm);

template <class I, class H>
static void delete_with_held_items(const I in, const logic_step step, const H handle) {
	if (handle) {
		deletion_queue q;
		q.push_back(handle.get_id());

		handle.for_each_contained_item_recursive(
			[&](const auto& contained) {
				if constexpr(std::is_same_v<arena_mode, typename I::mode_type>) {
					const auto& bomb_flavour = in.rules.bomb_flavour;

					if (bomb_flavour.is_set()) {
						if (entity_flavour_id(bomb_flavour) == entity_flavour_id(contained.get_flavour_id())) {
							/* Don't delete the bomb!!! Drop it instead. */

							auto request = item_slot_transfer_request::drop(contained);
							request.params.bypass_mounting_requirements = true;

							const auto result = perform_transfer_no_step(request, step.get_cosmos());
							result.notify_logical(step);

							return;
						}
					}
				}

				q.push_back(entity_id(contained.get_id()));
			}
		);

		reverse_perform_deletions(q, handle.get_cosmos());
	}
}

