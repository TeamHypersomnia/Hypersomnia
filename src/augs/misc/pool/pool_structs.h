#pragma once

namespace augs {
/*	
	We generate introspectors for the pool and its internal structures,
	becasuse it will be very useful to be able to dump the values to lua files,
	if some debugging is necessary.
*/

	template <class size_type>
	struct pool_slot {
		// GEN INTROSPECTOR struct augs::pool_slot class size_type
		size_type pointing_indirector = -1;
		// END GEN INTROSPECTOR
	};

	template <class size_type>
	struct pool_indirector {
		// GEN INTROSPECTOR struct augs::pool_indirector class size_type
		size_type real_index = static_cast<size_type>(-1);
		size_type version = 1;
		// END GEN INTROSPECTOR
	};

	template <class size_type, class... keys>
	struct pool_undo_free_input {
		size_type real_index = static_cast<size_type>(-1);
		size_type indirection_index = static_cast<size_type>(-1);
	};
}
