#include "game/cosmos/entity_id.h"
#include "game/cosmos/cosmos.h"

#include "input_system.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "augs/window_framework/event.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"

using namespace augs;

void input_system::make_input_messages(const logic_step step) {
	for (const auto& p : step.get_entropy().players) {
		for (const auto& intent : p.second.intents) {
			messages::intent_message msg;
			msg.game_intent::operator=(intent);
			msg.subject = p.first;
			step.post_message(msg);
		}

		for (const auto& motion : p.second.motions) {
			messages::motion_message msg;
			msg.game_motion::operator=(motion);
			msg.subject = p.first;
			step.post_message(msg);
		}
	}
}