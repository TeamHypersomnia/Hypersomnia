#include "augs/misc/machine_entropy.h"
#include "augs/templates/container_templates.h"

namespace augs {
#if 0
	machine_entropy& machine_entropy::operator+=(const machine_entropy& b) {
		concatenate(local, b.local);
		concatenate(remote, b.remote);

		return *this;
	}

	bool machine_entropy::operator==(const machine_entropy& b) const {
		return local == b.local && remote == b.remote;
	}

	void machine_entropy::apply_to(event::state&) const;

	bool machine_entropy::empty() const {
		return local.empty() && remote.empty();
	}

	void machine_entropy::clear() {
		local.clear();
		remote.clear();
	}
#endif
}