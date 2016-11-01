#include "cosmic_entropy.h"
#include "game/transcendental/cosmos.h"

#include "augs/misc/machine_entropy.h"
#include "game/global/input_context.h"

guid_mapped_entropy::guid_mapped_entropy(const cosmic_entropy& b, const cosmos& mapper) {
	for (const auto& entry : b.entropy_per_entity)
		entropy_per_entity[mapper[entry.first].get_guid()] = entry.second;
}

bool guid_mapped_entropy::operator!=(const guid_mapped_entropy& b) const {
	if (entropy_per_entity.size() != b.entropy_per_entity.size())
		return true;

	for (const auto& entry : b.entropy_per_entity) {
		auto found = entropy_per_entity.find(entry.first);
	
		if (found == entropy_per_entity.end())
			return true;
	
		if (entry.second != (*found).second)
			return true;
	}

	return false;
}

cosmic_entropy::cosmic_entropy(const guid_mapped_entropy& b, const cosmos& mapper) {
	for (const auto& entry : b.entropy_per_entity)
		entropy_per_entity[mapper.get_entity_by_guid(entry.first).get_id()] = entry.second;
}
