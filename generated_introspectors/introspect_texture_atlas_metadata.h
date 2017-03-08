#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(atlas_image_size));

		f(t.NVP(images));
		f(t.NVP(fonts));
	}

}