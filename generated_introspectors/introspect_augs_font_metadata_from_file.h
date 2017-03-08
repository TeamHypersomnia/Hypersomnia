#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(ascender));
		f(t.NVP(descender));

		f(t.NVP(pt));

		f(t.NVP(glyphs));
		f(t.NVP(unicode_to_glyph_index));
	}

}