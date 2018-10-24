#include "melee_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"

#include "game/components/melee_component.h"
#include "game/components/melee_fighter_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/torso_component.hpp"

#include "game/detail/frame_calculation.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/melee/like_melee.h"

using namespace augs;

namespace components {
	bool melee_fighter::now_returning() const {
		return state == melee_fighter_state::RETURNING || state == melee_fighter_state::CLASH_RETURNING;
	}

	bool melee_fighter::fighter_orientation_frozen() const {
		const bool allow_rotation = state == melee_fighter_state::READY || state == melee_fighter_state::COOLDOWN;
		return !allow_rotation;
	}
}

void melee_system::advance_thrown_melee_logic(const logic_step step) {
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::melee>([&](const auto& it) {
		auto& sender = it.template get<components::sender>();

		if (sender.is_set()) {
			if (!has_hurting_velocity(it)) {
				LOG_NVPS(it, it.get_effective_velocity());
				sender.unset();
				it.infer_rigid_body();
				it.infer_colliders();
			}
		}
	});
}

void melee_system::initiate_and_update_moves(const logic_step step) {
	auto& cosm = step.get_cosmos();
	//const auto anims = cosm.get_logical_assets().plain_animations;

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		//const auto& fighter_def = it.template get<invariants::melee_fighter>();
		//const auto& torso = it.template get<invariants::torso>();
		const auto& sentience = it.template get<components::sentience>();
		const auto wielded_items = it.get_wielded_items();
		//const auto stance = torso.calc_stance(it, it.get_wielded_items());

		//auto& fighter = it.template get<components::melee_fighter>();

		entity_id target_weapon;

		const auto chosen_action = [&]() {
			if (wielded_items.size() != 1) {
				/* Disallow any initiation if we're not wielding one and only one melee weapon */
				return weapon_action_type::COUNT;
			}

			/* 
				We may have both mouse buttons at the same time, 
				but we must prioritize either button. 

				Always choose primary over the secondary.
			*/

			for (std::size_t i = 0; i < hand_count_v; ++i) {
				const auto& hand_flag = sentience.hand_flags[i];

				if (hand_flag) {
					const auto action = it.calc_hand_action(i);

					if (cosm[action.held_item].template has<components::melee>()) {
						target_weapon = action.held_item;
						return action.type;
					}
				}
			}

			return weapon_action_type::COUNT;
		}();
		(void)chosen_action;

#if 0
		if (chosen_action != weapon_action_type::COUNT) {
			cosm[target_weapon].dispatch_on_having<components::melee>(
				[&](const auto& typed_weapon) {

				}
			);
		}

		const auto stance_id = ::calc_stance_id(it, wielded_items);

		if (fighter.state == melee_fighter_state::READY) {
			auto try_init_action = [&](const auto& type) {
				auto& input_flags = fighter.input_flags;

				if (fighter.input_flags[type]) {
					fighter.state = melee_fighter_state.IN_ACTION;
					fighter.action = type;
					return true;
				}

				return false;
			};

			if (!try_init_action(weapon_action_type::PRIMARY)) {
				try_init_action(weapon_action_type::SECONDARY);
			}

			return;
		}

		if (fighter.state == melee_fighter_state::IN_ACTION) {
			const auto info = ::calc_stance_usage(
				cosm,
				torso.stances[stance_id],
				{},
				wielded_items
			);
		}

		if (fighter.state == melee_fighter_state::COOLDOWN) {

		}
#endif
	});
}