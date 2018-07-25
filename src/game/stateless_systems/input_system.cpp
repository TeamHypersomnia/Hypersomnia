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
	for (const auto& per_entity : step.get_entropy().intents_per_entity) {
		for (const auto& raw : per_entity.second) {
			messages::intent_message intent;
			intent.game_intent::operator=(raw);
			intent.subject = per_entity.first;
			step.post_message(intent);
		}
	}

	for (const auto& per_entity : step.get_entropy().motions_per_entity) {
		for (const auto& raw : per_entity.second) {
			messages::motion_message motion;
			motion.game_motion::operator=(raw);
			motion.subject = per_entity.first;
			step.post_message(motion);
		}
	}
}