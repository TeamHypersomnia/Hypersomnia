#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(pickup_timeout));
		f(t.NVP(mounting));

		f(t.NVP(only_pick_these_items));
		f(t.NVP(pick_all_touched_items_if_list_to_pick_empty));
	}

}