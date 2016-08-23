#include <random>

#include "augs/window_framework/window.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"

#include "input_system.h"

#include "augs/templates.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/time_utils.h"

#include "game/components/transform_component.h"
#include "game/components/gun_component.h"

#include "game/messages/intent_message.h"

#include "game/messages/gui_intents.h"

#include "augs/window_framework/event.h"
#include "augs/misc/step_player.h"
#include "game/transcendental/step.h"
#include "game/transcendental/entity_handle.h"

using namespace augs::window;

void input_system::make_intent_messages(fixed_step& step) {
	auto& cosmos = step.cosm;

	for (const auto& per_entity : step.entropy.entropy_per_entity) {
		for (const auto& raw : per_entity.second) {
			messages::intent_message intent;
			intent.entity_intent::operator=(raw);
			intent.subject = per_entity.first;
			step.messages.post(intent);
		}
	}
}