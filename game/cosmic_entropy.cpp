#include "cosmic_entropy.h"
#include "templates.h"

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

