#include "filters.h"

auto all = std::numeric_limits<decltype(b2Filter::categoryBits)>::max() & (~filters::TRIGGER);

namespace filters {
	b2Filter renderable() {
		b2Filter out;
		out.categoryBits = RENDERABLE;
		out.maskBits = RENDERABLE_QUERY;
		return out;
	}

	b2Filter renderable_query() {
		b2Filter out;
		out.categoryBits = RENDERABLE_QUERY;
		out.maskBits = RENDERABLE;
		return out;
	}
	
	b2Filter controlled_character() {
		b2Filter out;
		out.categoryBits = RENDERABLE | CONTROLLED_CHARACTER | DYNAMIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | TRIGGER;
		return out;
	}

	b2Filter dynamic_object() {
		b2Filter out;
		out.categoryBits = RENDERABLE | DYNAMIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND;
		return out;
	}

	b2Filter static_object() {
		b2Filter out;
		out.categoryBits = RENDERABLE | STATIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND;
		return out;
	}

	b2Filter trigger() {
		b2Filter out;
		out.categoryBits = RENDERABLE | TRIGGER;
		out.maskBits = RENDERABLE_QUERY | TRIGGER | CONTROLLED_CHARACTER;
		return out;
	}
}