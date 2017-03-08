#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(activated));

		f(t.NVP(processing_subject_categories));
		f(t.NVP(disabled_categories));
	}

}