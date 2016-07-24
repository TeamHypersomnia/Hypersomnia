#include "cosmic_entropy.h"
#include "augs/templates.h"
#include "game/cosmos.h"
#include "game/components/input_receiver_component.h"
#include "augs/misc/machine_entropy.h"

cosmic_entropy& cosmic_entropy::operator+=(const cosmic_entropy& b) {
	for (auto& ent : b.entropy_per_entity) {
		auto& vec = entropy_per_entity[ent.first];
		vec.insert(vec.end(), ent.second.begin(), ent.second.end());
	}

	return *this;
}

size_t cosmic_entropy::length() const {
	size_t total = 0;

	for (auto& ent : entropy_per_entity) {
		total += ent.second.size();
	}

	return total;
}

void cosmic_entropy::from_input_receivers_distribution(const augs::machine_entropy& machine, cosmos& cosm) {
	auto targets = cosm.get(processing_subjects::WITH_INPUT_RECEIVER);

	for (auto it : targets) {
		if (it.get<components::input_receiver>().local) {
			cosmic_entropy new_entropy;
			new_entropy.entropy_per_entity[it] = machine.local;

			(*this) += new_entropy;
		}
	}
}