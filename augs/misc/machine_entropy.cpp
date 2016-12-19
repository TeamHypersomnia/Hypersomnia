#include "machine_entropy.h"
#include "augs/templates/container_templates.h"

namespace augs {
	machine_entropy& machine_entropy::operator+=(const machine_entropy& b) {
		local.insert(local.end(), b.local.begin(), b.local.end());
		remote.insert(remote.end(), b.remote.begin(), b.remote.end());

		return *this;
	}

	bool machine_entropy::operator==(const machine_entropy& b) const {
		return compare_containers(local, b.local) && compare_containers(remote, b.remote);
	}

	bool machine_entropy::empty() const {
		return local.empty() && remote.empty();
	}

	void machine_entropy::clear() {
		local.clear();
		remote.clear();
	}
}