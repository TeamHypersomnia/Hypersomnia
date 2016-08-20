#include "machine_entropy.h"
#include "augs/templates.h"

namespace augs {
	machine_entropy& machine_entropy::operator+=(const machine_entropy& b) {
		local.insert(local.end(), b.local.begin(), b.local.end());
		remote.insert(remote.end(), b.remote.begin(), b.remote.end());

		return *this;
	}

	bool machine_entropy::empty() const {
		return local.empty() && remote.empty();
	}

}