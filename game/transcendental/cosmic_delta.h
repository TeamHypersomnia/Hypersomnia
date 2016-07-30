#pragma once
#include <vector>
#include "entity_id.h"

class cosmos;

class cosmic_delta {
	std::vector<entity_id> deleted;
	std::vector<entity_id> changed;
	std::vector<entity_id> created;

public:
	cosmic_delta(cosmos&, cosmos&);

	template <class Archive>
	void serialize(Archive& ar) {
		ar(deleted, changed, created);
	}
};