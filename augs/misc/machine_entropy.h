#pragma once
#include <vector>

#include "augs/window_framework/event.h"
#include "augs/network/network_types.h"

namespace augs {
	struct machine_entropy {
		typedef std::vector<augs::window::event::change> local_type;
		typedef std::vector<augs::network::message> remote_type;

		// GEN INTROSPECTOR struct augs::machine_entropy
		local_type local;
		remote_type remote;
		// END GEN INTROSPECTOR

		// here will be remote entropy as well

		machine_entropy& operator+=(const machine_entropy&);
		bool operator==(const machine_entropy&) const;
		bool empty() const;
		void clear();
	};
}