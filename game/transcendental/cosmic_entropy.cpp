#include "cosmic_entropy.h"
#include "augs/templates.h"
#include "game/transcendental/cosmos.h"
#include "game/components/input_receiver_component.h"
#include "augs/misc/machine_entropy.h"
#include "game/global/input_context.h"

cosmic_entropy& cosmic_entropy::operator+=(const cosmic_entropy& b) {
	for (const auto& ent : b.entropy_per_entity) {
		auto& vec = entropy_per_entity[ent.first];
		vec.insert(vec.end(), ent.second.begin(), ent.second.end());
	}

	return *this;
}

size_t cosmic_entropy::length() const {
	size_t total = 0;

	for (const auto& ent : entropy_per_entity)
		total += ent.second.size();

	return total;
}

void cosmic_entropy::from_input_receivers_distribution(const augs::machine_entropy& machine, const input_context& input, cosmos& cosm) {
	ensure(entropy_per_entity.empty());
	
	auto targets = cosm.get(processing_subjects::WITH_INPUT_RECEIVER);

	for (const auto& it : targets) {
		if (it.get<components::input_receiver>().local) {
			auto& intents = entropy_per_entity[it];

			for (const auto& raw : machine.local) {
				entity_intent mapped;

				if (make_intent(input, raw, mapped))
					intents.push_back(mapped);
			}
		}
	}
}

bool cosmic_entropy::make_intent(const input_context& context, const augs::window::event::state& raw, entity_intent& mapped_intent) {
	intent_type intent;

	bool found_context_entry = false;

	if (raw.key_event == augs::window::event::NO_CHANGE) {
		mapped_intent.pressed_flag = true;

		const auto found_intent = context.event_to_intent.find(raw.msg);
		if (found_intent != context.event_to_intent.end()) {
			intent = (*found_intent).second;
			found_context_entry = true;
		}
	}
	else if (raw.key_event == augs::window::event::PRESSED || raw.key_event == augs::window::event::RELEASED) {
		mapped_intent.pressed_flag = raw.key_event == augs::window::event::PRESSED;

		const auto found_intent = context.key_to_intent.find(raw.key);
		if (found_intent != context.key_to_intent.end()) {
			intent = (*found_intent).second;
			found_context_entry = true;
		}
	}

	mapped_intent.intent = intent;
	mapped_intent.mouse_rel = raw.mouse.rel;
	return found_context_entry;
}