#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(parent_slot));
		f(t.NVP(current_address));
		f(t.NVP(attachment_offset));
		f(t.NVP(item_remains_physical));
	}

}