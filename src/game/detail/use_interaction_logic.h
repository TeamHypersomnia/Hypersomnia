#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/hand_fuse_logic.h"
#include "game/enums/interaction_result_type.h"
#include "game/detail/melee/like_melee.h"
#include "game/detail/entity_handle_mixins/find_target_slot_for.hpp"
#include "game/enums/filters.h"
#include "game/inferred_caches/physics_world_cache.hpp"

struct bomb_defuse_interaction {
	entity_id bomb_subject;
	interaction_result_type progress = interaction_result_type::NOTHING_FOUND;

	bool is_in_progress() const {
		return progress == interaction_result_type::IN_PROGRESS;
	}

	bool can_begin_interaction() const {
		return progress == interaction_result_type::CAN_BEGIN;
	}

	interaction_result_type process(const logic_step step, const entity_id character) const {
		if (can_begin_interaction()) {
			step.get_cosmos()[bomb_subject].template get<components::hand_fuse>().character_now_defusing = character;
		}

		return progress;
	}
};

struct item_pickup {
	entity_id item;

	interaction_result_type process(const logic_step step, const entity_id character) const {
		auto& cosm = step.get_cosmos();
		const auto& clk = cosm.get_clock();
		const auto picker = cosm[character];

		auto handle_pickup = [&](const auto& typed_item) {
			picker.dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_picker) {
				auto& transfers = typed_picker.template get<components::item_slot_transfers>();
				const auto& pick_list = transfers.only_pick_these_items;
				const bool found_on_subscription_list = found_in(pick_list, typed_item.get_id());

				if (/* item_subscribed */
					(pick_list.empty() && transfers.pick_all_touched_items_if_list_to_pick_empty)
					|| found_on_subscription_list
				) {
					const bool can_pick_already = transfers.pickup_timeout.try_to_fire_and_reset(clk);

					if (!can_pick_already) {
						return;
					}

					const auto pickup_slot = typed_picker.find_pickup_target_slot_for(typed_item, { slot_finding_opt::OMIT_MOUNTED_SLOTS });

					if (pickup_slot.alive()) {
						perform_transfer(item_slot_transfer_request::standard(entity_id(typed_item.get_id()), pickup_slot), step);
					}
				}
			});
		};

		cosm[item].dispatch_on_having_all<invariants::item>(handle_pickup);

		return interaction_result_type::CAN_BEGIN;
	}
};

using use_interaction_variant = std::variant<
	item_pickup,
	bomb_defuse_interaction
>;

template <class E>
std::optional<use_interaction_variant> query_use_interaction(const E& subject) {
	if (!subject.template has<invariants::sentience>()) {
		return std::nullopt;
	}

	auto& cosm = subject.get_cosmos();
	const auto max_defuse_radius = subject.template get<invariants::sentience>().interaction_hitbox_radius;
	const auto where = subject.get_logic_transform().pos;

	auto& entities = thread_local_visible_entities();

	entities.reacquire_all({
		cosm,
		camera_cone(camera_eye(where, 1.f), vec2i::square(max_defuse_radius * 2)),
		accuracy_type::EXACT,
		render_layer_filter::whitelist(render_layer::PLANTED_ITEMS, render_layer::ITEMS_ON_GROUND),
		{ { tree_of_npo_type::RENDERABLES } }
	});

	/* Important for the sake of determinism */
	entities.sort(cosm);

	std::vector<bomb_defuse_interaction> viable_defuses;

	entities.for_each<render_layer::PLANTED_ITEMS>(cosm, [&](const auto& bomb_handle) {
		bomb_handle.template dispatch_on_having_all<components::hand_fuse>([&](const auto typed_bomb) -> callback_result {
			const auto& fuse = typed_bomb.template get<components::hand_fuse>();
			const auto character_now_defusing = cosm[fuse.character_now_defusing];
			const bool already_busy = character_now_defusing.alive() && character_now_defusing != subject;

			if (already_busy) {
				return callback_result::CONTINUE;
			}

			const bool already_us = fuse.character_now_defusing == subject.get_id();
			const auto fuse_logic = stepless_fuse_logic_provider(typed_bomb);

			if (fuse_logic.defusing_character_in_range(subject)) {
				auto defuse_interaction = bomb_defuse_interaction();

				defuse_interaction.bomb_subject = typed_bomb;
				defuse_interaction.progress = interaction_result_type::IN_RANGE_BUT_CANT;

				if (fuse_logic.defusing_conditions_fulfilled(subject)) {
					defuse_interaction.progress = interaction_result_type::CAN_BEGIN;

					if (already_us) {
						defuse_interaction.progress = interaction_result_type::IN_PROGRESS;
					}
				}

				viable_defuses.push_back(defuse_interaction);
			}

			return callback_result::CONTINUE;
		});
	});


	if (!viable_defuses.empty()) {
		/* 
			TODO: take all viable bombs and take the best interaction in order: 
			current->closest viable->any in range. That would be the most correct.

			For now we take any that is in range because that will always be the only one.

			If any bomb is at all in range then prioritize that over picking up any items.
		*/

		const auto& best_defuse = viable_defuses[0];
		return best_defuse;
	}

	thread_local auto overlaps = std::vector<b2TestOverlapOutput>();
	overlaps.clear();

	auto best_item = entity_id();
	auto best_distance = 0.0f;

	//const auto si = cosm.get_si();
	//const auto query_center = ::get_interaction_query_center(subject);
	const auto query_top = ::get_interaction_query_top(subject);

	const auto transform = subject.get_logic_transform();
	const auto segment_a = transform.pos; 
	const auto segment_b = query_top; 

	entities.for_each<render_layer::ITEMS_ON_GROUND>(cosm, [&](const const_entity_handle& touched_item_part) {
		auto handle_candidate = [&](const auto& typed_root_item) {
			{
				const auto& clk = cosm.get_clock();

				auto c = typed_root_item.get_special_physics().dropped_or_created_cooldown;
				c.cooldown_duration_ms = 80.f;

				if (!c.is_ready(clk)) {
					return;
				}
			}

			if (is_like_thrown_melee(typed_root_item)) {
				if (const auto sender = typed_root_item.template find<components::sender>()) {
					if (!sender->is_sender_subject(subject)) {
						return;
					}
				}
			}

			if (typed_root_item.get_owning_transfer_capability().alive()) {
				/* Item is somehow used by someone else */
				return;
			}

			if (::calc_filters(typed_root_item) != filters[predefined_filter_type::LYING_ITEM]) {
				/* 
					Only match objects with physics of lying items.
					Otherwise we might pick up a defused bomb, for example.
				*/

				return;
			}

			if (const auto overlap = ::interaction_hitbox_overlaps(subject, touched_item_part)) {
				const auto item_center = touched_item_part.get_logic_transform().pos;

				//const auto overlap_location = vec2(si.get_pixels(overlap->pointA));
				//const auto distance = (overlap_location - query_center).length_sq();
				const auto distance = item_center.sq_distance_from_segment(segment_a, segment_b); 

				if (!best_item.is_set() || distance < best_distance) {
					best_item = typed_root_item.get_id();
					best_distance = distance;
				}
			}
		};

		{
			const auto& physics = cosm.get_solvable_inferred().physics;
			const auto ray_a = transform.pos;
			const auto ray_b = touched_item_part.get_logic_transform().pos;

			const auto ray = physics.ray_cast_px(
				cosm.get_si(),
				ray_a,
				ray_b,
				predefined_queries::pathfinding(),
				touched_item_part
			);

			if (ray.hit) {
				return;
			}
		}

		const const_entity_handle considered_root = [&]() {
			if (touched_item_part.get_current_slot().alive()) {
				return touched_item_part.get_current_slot().get_root_container();
			}

			return touched_item_part;
		}();

		considered_root.dispatch_on_having_all<invariants::item>(handle_candidate);
	});

	if (best_item.is_set()) {
		return item_pickup { best_item };
	}

	return std::nullopt;
}
