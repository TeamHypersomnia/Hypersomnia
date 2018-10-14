#include "game/stateless_systems/input_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

void input_system::make_input_messages(const logic_step step) {
	for (const auto& p : step.get_entropy().players) {
		for (const auto& intent : p.second.intents) {
			auto msg = messages::intent_message();
			msg.game_intent::operator=(intent);
			msg.subject = p.first;
			step.post_message(msg);
		}

		for (const auto& motion : p.second.motions) {
			auto msg = messages::motion_message();
			msg.game_motion::operator=(motion);
			msg.subject = p.first;
			step.post_message(msg);
		}
	}
}