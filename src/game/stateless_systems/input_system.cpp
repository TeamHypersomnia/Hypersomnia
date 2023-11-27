#include "game/stateless_systems/input_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/messages/changed_identities_message.h"

void input_system::make_input_messages(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& changed_ids_queue = step.get_queue<messages::changed_identities_message>();

	for (const auto& p : step.get_entropy().players) {
		auto considered_id = p.first;

		for (const auto& c : changed_ids_queue) {
			if (const auto new_id = mapped_or_nullptr(c.changes, considered_id)) {
				considered_id = *new_id;
			}
		}

		const auto subject = cosm[considered_id];

		if (subject.dead()) {
			continue;
		}

		const auto& commands = p.second.commands;
		const auto& settings = p.second.settings;

		if (const auto movement = subject.template find<components::movement>()) {
			movement->keep_movement_forces_relative_to_crosshair = settings.keep_movement_forces_relative_to_crosshair;
		}

		for (const auto& intent : commands.intents) {
			auto msg = messages::intent_message();
			msg.game_intent::operator=(intent);
			msg.subject = considered_id;
			step.post_message(msg);
		}

		for (const auto motion : commands.motions) {
			auto msg = messages::motion_message();

			const auto type = motion.first;

			msg.motion = type;
			msg.offset = vec2(motion.second) * settings.crosshair_sensitivity;
			msg.subject = considered_id;
			step.post_message(msg);
		}
	}
}