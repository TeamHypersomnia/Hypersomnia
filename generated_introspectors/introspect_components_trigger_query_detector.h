#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(detection_intent_enabled));
		f(t.NVP(spam_trigger_requests_when_detection_intented));
	}

}