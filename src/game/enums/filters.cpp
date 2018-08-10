#include "game/enums/filters.h"
#include "augs/misc/enum/enum_bitset.h"

using C = filters::categories;

template <class... Args>
static auto make_flags(Args... enums) {
	return static_cast<uint16>(augs::enum_bitset<C>(enums...).to_ulong());
}

static auto standard_participation_bitset() {
	return augs::enum_bitset<C>(C::QUERY, C::FLYING, C::CHARACTER, C::LYING_ITEM, C::WALL, C::GROUND, C::TRIGGER, C::BULLET, C::SHELL, C::GLASS_OBSTACLE);
}

static auto standard_participation() {
	return static_cast<uint16>(standard_participation_bitset().to_ulong());
}

static auto standard_participation_except(C c) {
	auto r = standard_participation_bitset();
	r.reset(c);
	return static_cast<uint16>(r.to_ulong());
}

namespace filters {
	b2Filter line_of_sight_query() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE);
		return out;
	}

	b2Filter pathfinding_query() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE);
		return out;
	}
	
	b2Filter renderable_query() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::BULLET, C::CHARACTER, C::LYING_ITEM, C::WALL, C::GROUND, C::SHELL, C::GLASS_OBSTACLE, C::FLYING);
		return out;
	}

	b2Filter wall() {
		b2Filter out;
		out.categoryBits = make_flags(C::WALL);
		out.maskBits = standard_participation();
		return out;
	}

	b2Filter character() {
		b2Filter out;
		out.categoryBits = make_flags(C::CHARACTER);
		out.maskBits = standard_participation();
		return out;
	}
	
	b2Filter ground() {
		b2Filter out;
		out.categoryBits = make_flags(C::GROUND);
		out.maskBits = standard_participation_except(C::FLYING);
		return out;
	}

	b2Filter lying_item() {
		b2Filter out;
		out.categoryBits = make_flags(C::LYING_ITEM);
		out.maskBits = standard_participation();
		return out;
	}

	b2Filter flying_item() {
		b2Filter out;
		out.categoryBits = make_flags(C::FLYING);
		out.maskBits = standard_participation_except(C::LYING_ITEM);
		return out;
	}

	b2Filter flying_bullet() {
		b2Filter out;
		out.categoryBits = make_flags(C::FLYING);
		out.maskBits = standard_participation();
		return out;
	}

	b2Filter shell() {
		b2Filter out;
		out.categoryBits = make_flags(C::SHELL);
		out.maskBits = standard_participation_except(C::FLYING);
		return out;
	}

	b2Filter planted_explosive() {
		b2Filter out;
		out.categoryBits = make_flags(C::LYING_ITEM);
		out.maskBits = make_flags(C::QUERY, C::SHELL);
		return out;
	}

	b2Filter glass_obstacle() {
		b2Filter out;
		out.categoryBits = make_flags(C::GLASS_OBSTACLE);
		out.maskBits = standard_participation();
		return out;
	}

}
