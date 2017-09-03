#pragma once
#include <vector>

#include "augs/window_framework/event.h"
#include "augs/network/network_types.h"

namespace augs {
	using local_entropy = std::vector<event::change>;
	using remote_entropy = std::vector<event::change>;

#if 0
	struct machine_entropy {
		using local_type = std::vector<augs::event::change>;
		using remote_type = std::vector<augs::network::message>;

		// GEN INTROSPECTOR struct augs::machine_entropy
		local_type local;
		remote_type remote;
		// END GEN INTROSPECTOR

		// here will be remote entropy as well

		machine_entropy& operator+=(const machine_entropy&);
		
		void clear();

		void apply_to(event::state&) const;
		bool operator==(const machine_entropy&) const;
		bool empty() const;
	};
#endif
}