#pragma once
#include "augs/window_framework/event.h"
#include "augs/network/network_types.h"
#include <vector>

namespace augs {
	struct machine_entropy {
		typedef std::vector<augs::window::event::change> local_type;
		typedef std::vector<augs::network::message> remote_type;

		local_type local;
		remote_type remote;
		// here will be remote entropy as well

		machine_entropy& operator+=(const machine_entropy&);
		bool operator==(const machine_entropy&) const;
		bool empty() const;
		void clear();
	};
}

namespace augs {
	template<class A>
	bool read_object(A& ar, machine_entropy& s) {
		if(!read_object(ar, s.local)) return false;
		return read_vector_of_objects(ar, s.remote);
	}

	template<class A>
	void write_object(A& ar, const machine_entropy& s) {
		write_object(ar, s.local);
		write_vector_of_objects(ar, s.remote);
	}
}