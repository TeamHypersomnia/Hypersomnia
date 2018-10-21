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

using namespace augs;

void melee_system::consume_melee_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		auto* const maybe_melee = cosm[it.subject].find<components::melee_fighter>();
		
		if (maybe_melee == nullptr) {
			continue;
		}

		auto& melee = *maybe_melee;

		if (it.intent == game_intent_type::MELEE_PRIMARY_MOVE) {
			melee.flags.primary = it.was_pressed();
		}

		if (it.intent == game_intent_type::MELEE_SECONDARY_MOVE) {
			melee.flags.secondary = it.was_pressed();
		}

		if (it.intent == game_intent_type::MELEE_BLOCK) {
			melee.flags.block = it.was_pressed();
		}
	}
}

void melee_system::initiate_and_update_moves(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& anims = cosm.get_logical_assets().plain_animations;

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		const auto& fighter_def = it.template get<invariants::melee_fighter>();
		const auto& torso = it.template get<invariants::torso>();
		const auto stance = torso.calc_stance(it, it.get_wielded_items());

		auto& fighter = it.template get<components::melee_fighter>();

		const auto wielded_items = it.get_wielded_items();
		const auto stance_id = ::calc_stance_id(it, wielded_items);

		if (fighter.state == melee_fighter_state::READY) {
			auto try_init_action = [&](const auto& type) {
				if (fighter.flags[type]) {
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
	};
}