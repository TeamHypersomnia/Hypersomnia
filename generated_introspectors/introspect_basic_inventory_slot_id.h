#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F, class id_type>
	void introspect(
		maybe_const_ref_t<C, basic_inventory_slot_id<id_type>> t,
		F f
	) {
		f(t.NVP(type));
		f(t.NVP(container_entity));
	}

}