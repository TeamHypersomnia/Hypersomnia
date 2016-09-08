#include "cosmic_entropy.h"
#include "augs/templates.h"
#include "game/transcendental/cosmos.h"

#include "augs/misc/machine_entropy.h"
#include "game/global/input_context.h"

bool make_entity_intent(const input_context& context, const augs::window::event::state& raw, entity_intent& mapped_intent) {
	bool found_context_entry = false;

	if (raw.key_event == augs::window::event::NO_CHANGE) {
		mapped_intent.pressed_flag = true;

		const auto found_intent = context.event_to_intent.find(raw.msg);
		if (found_intent != context.event_to_intent.end()) {
			mapped_intent.intent = (*found_intent).second;
			found_context_entry = true;
		}
	}
	else if (raw.key_event == augs::window::event::PRESSED || raw.key_event == augs::window::event::RELEASED) {
		mapped_intent.pressed_flag = raw.key_event == augs::window::event::PRESSED;

		const auto found_intent = context.key_to_intent.find(raw.key);
		if (found_intent != context.key_to_intent.end()) {
			mapped_intent.intent = (*found_intent).second;
			found_context_entry = true;
		}
	}

	mapped_intent.mouse_rel = raw.mouse.rel;
	return found_context_entry;
}

guid_mapped_entropy::guid_mapped_entropy(const cosmic_entropy& b, const cosmos& mapper) {
	delta_to_apply = b.delta_to_apply;

	for (const auto& entry : b.entropy_per_entity)
		entropy_per_entity[mapper[entry.first].get_guid()] = entry.second;
}

bool guid_mapped_entropy::operator!=(const guid_mapped_entropy& b) const {
	if (delta_to_apply != b.delta_to_apply)
		return true;

	if (entropy_per_entity.size() != b.entropy_per_entity.size())
		return true;

	for (const auto& entry : b.entropy_per_entity) {
		auto found = entropy_per_entity.find(entry.first);
	
		if (found == entropy_per_entity.end())
			return true;
	
		if (entry.second.size() != (*found).second.size())
			return true;

		for (size_t i = 0; i < entry.second.size(); ++i)
			if (entry.second[i] != (*found).second[i])
				return true;
	}

	return false;
}

cosmic_entropy::cosmic_entropy(const guid_mapped_entropy& b, const cosmos& mapper) {
	delta_to_apply = b.delta_to_apply;

	for (const auto& entry : b.entropy_per_entity)
		entropy_per_entity[mapper.get_entity_by_guid(entry.first).get_id()] = entry.second;
}
