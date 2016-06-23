#include "machine_entropy.h"
#include "templates.h"

namespace augs {
	machine_entropy& machine_entropy::operator+=(const machine_entropy& b) {
		local.insert(local.end(), b.local.begin(), b.local.end());
	}
}