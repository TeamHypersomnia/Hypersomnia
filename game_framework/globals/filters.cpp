#include "filters.h"

auto all = std::numeric_limits<decltype(b2Filter::categoryBits)>::max();;

namespace filters {
	b2Filter none() {
		b2Filter out;
		out.categoryBits = all;
		out.maskBits = 0;
		return out;
	}

	b2Filter controlled_character() {
		b2Filter out;
		out.categoryBits = all;
		out.maskBits = CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER;
		return out;
	}

	b2Filter dynamic_object() {
		b2Filter out;
		out.categoryBits = all;
		out.maskBits = all;
		return out;
	}

	b2Filter static_object() {
		b2Filter out;
		out.categoryBits = all;
		out.maskBits = all;
		return out;
	}
}