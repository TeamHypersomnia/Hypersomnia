#include "particles_existence_component.h"

using namespace augs;

namespace components {
	bool particles_existence::operator==(const particles_existence& b) const {
		return !std::memcmp(this, &b, sizeof(b));
	}

	bool particles_existence::operator!=(const particles_existence& b) const {
		return !operator==(b);
	}
}