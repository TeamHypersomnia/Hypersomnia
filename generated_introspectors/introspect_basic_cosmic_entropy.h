#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F, class key>
	void introspect(
		maybe_const_ref_t<C, basic_cosmic_entropy<key>> t,
		F f
	) {
		f(t.NVP(cast_spells));
		f(t.NVP(intents_per_entity));
		f(t.NVP(transfer_requests));
	}

}