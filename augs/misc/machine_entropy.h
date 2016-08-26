#pragma once
#include "augs/window_framework/event.h"
#include "augs/network/network_types.h"
#include <vector>

namespace augs {
	struct machine_entropy {
		typedef std::vector<augs::window::event::state> local_type;
		typedef std::vector<augs::network::message> remote_type;

		local_type local;
		remote_type remote;
		// here will be remote entropy as well

		machine_entropy& operator+=(const machine_entropy&);
		bool empty() const;
	};
}

namespace augs {
	template<class A>
	void read_object(A& ar, machine_entropy& s) {
		read_object(ar, s.local);
		read_vector_of_objects(ar, s.remote);
	}

	template<class A>
	void write_object(A& ar, const machine_entropy& s) {
		write_object(ar, s.local);
		write_vector_of_objects(ar, s.remote);
	}
}