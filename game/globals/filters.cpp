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
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | TRIGGER | BULLET | SHELL | CORPSE;
		return out;
	}
	
	b2Filter corpse() {
		b2Filter out;
		out.categoryBits = RENDERABLE | CORPSE;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | TRIGGER | SHELL | CORPSE;
		return out;
	}

	b2Filter friction_ground() {
		b2Filter out;
		out.categoryBits = RENDERABLE | FRICTION_GROUND;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | SHELL | CORPSE;
		return out;
	}

	b2Filter dynamic_object() {
		b2Filter out;
		out.categoryBits = RENDERABLE | DYNAMIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | BULLET | SHELL | CORPSE;
		return out;
	}

	b2Filter static_object() {
		b2Filter out;
		out.categoryBits = RENDERABLE | STATIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | BULLET | SHELL | CORPSE;
		return out;
	}

	b2Filter shell() {
		b2Filter out;
		out.categoryBits = RENDERABLE | SHELL;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER | FRICTION_GROUND | SHELL | CORPSE;
		return out;
	}

	b2Filter bullet() {
		b2Filter out;
		out.categoryBits = RENDERABLE | BULLET;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | REMOTE_CHARACTER;
		return out;
	}

	b2Filter trigger() {
		b2Filter out;
		out.categoryBits = RENDERABLE | TRIGGER;
		out.maskBits = RENDERABLE_QUERY | TRIGGER | CONTROLLED_CHARACTER;
		return out;
	}
}