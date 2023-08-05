#include "game/enums/filters.h"
#include "augs/misc/enum/enum_bitset.h"

using C = filter_category;

const predefined_filters filters;

template <class... Args>
static auto make_flags(Args... enums) {
	return static_cast<uint16>(augs::enum_bitset<C>(enums...).to_ulong());
}

static auto standard_participation_bitset() {
	return augs::enum_bitset<C>(
		C::QUERY,
		C::WALL,
		C::CHARACTER,
		C::CHARACTER_WEAPON,
		C::LYING_ITEM,
		C::CAR_FLOOR,
		C::FLYING_BULLET,
		C::FLYING_EXPLOSIVE,
		C::FLYING_MELEE,
		C::SHELL,
		C::GLASS_OBSTACLE
	);
}

static auto standard_participation() {
	return static_cast<uint16>(standard_participation_bitset().to_ulong());
}

template <class... C>
static auto standard_participation_except(C... c) {
	auto r = standard_participation_bitset();
	(r.reset(c), ...);
	return static_cast<uint16>(r.to_ulong());
}

namespace predefined_queries {
	b2Filter line_of_sight() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL);
		return out;
	}

	b2Filter melee_query() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE, C::CHARACTER, C::LYING_ITEM, C::FLYING_EXPLOSIVE, C::FLYING_MELEE);
		return out;
	}

	b2Filter melee_solid_obstacle_query() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE);
		return out;
	}

	b2Filter pathfinding() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE);
		return out;
	}
	
	b2Filter force_explosion() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = standard_participation_except(C::FLYING_BULLET, C::LYING_ITEM, C::SHELL);
		return out;
	}

	b2Filter renderable() {
		b2Filter out;
		out.categoryBits = make_flags(C::QUERY);
		out.maskBits = make_flags(
			C::QUERY,
			C::WALL,
			C::CHARACTER,
			C::LYING_ITEM,
			C::CAR_FLOOR,
			C::FLYING_BULLET,
			C::FLYING_EXPLOSIVE,
			C::FLYING_MELEE,
			C::SHELL,
			C::GLASS_OBSTACLE
		);

		return out;
	}
}

predefined_filters::predefined_filters() {
	{
		auto& out = filters[predefined_filter_type::WALL];
		out.categoryBits = make_flags(C::WALL);
		out.maskBits = standard_participation();
	}
	{
		auto& out = filters[predefined_filter_type::CHARACTER];
		out.categoryBits = make_flags(C::CHARACTER);
		out.maskBits = standard_participation_except();
	}
	{
		auto& out = filters[predefined_filter_type::DEAD_CHARACTER];
		out.categoryBits = make_flags(C::CHARACTER);
		out.maskBits = standard_participation_except(C::CHARACTER, C::CHARACTER_WEAPON);
	}
	{

		auto& out = filters[predefined_filter_type::CHARACTER_WEAPON];
		out.categoryBits = make_flags(C::CHARACTER_WEAPON);
		out.maskBits = standard_participation_except(C::CHARACTER_WEAPON, C::CHARACTER);
	}
	{

		auto& out = filters[predefined_filter_type::CAR_FLOOR];
		out.categoryBits = make_flags(C::CAR_FLOOR);
		out.maskBits = standard_participation_except(C::FLYING_EXPLOSIVE, C::FLYING_BULLET, C::FLYING_MELEE);
	}
	{

		auto& out = filters[predefined_filter_type::LYING_ITEM];
		out.categoryBits = make_flags(C::LYING_ITEM);
		out.maskBits = standard_participation_except(C::CHARACTER, C::CHARACTER_WEAPON);
	}
	{
		auto& out = filters[predefined_filter_type::FLYING_BULLET];
		out.categoryBits = make_flags(C::FLYING_BULLET);
		out.maskBits = standard_participation_except(C::LYING_ITEM, C::FLYING_BULLET);
	}
	{
		auto& out = filters[predefined_filter_type::PENETRATING_BULLET];
		out.categoryBits = make_flags(C::FLYING_BULLET);
		out.maskBits = standard_participation_except(C::LYING_ITEM, C::FLYING_BULLET, C::WALL, C::GLASS_OBSTACLE);
	}
	{
		auto& out = filters[predefined_filter_type::PENETRATING_PROGRESS_QUERY];
		out.categoryBits = make_flags(C::FLYING_BULLET);
		out.maskBits = make_flags(C::WALL, C::GLASS_OBSTACLE);
	}
	{

		/* Rockets should collide with characters */
		auto& out = filters[predefined_filter_type::FLYING_ROCKET];
		out.categoryBits = make_flags(C::FLYING_EXPLOSIVE);
		out.maskBits = standard_participation_except(C::LYING_ITEM, C::FLYING_EXPLOSIVE);
	}
	{

		auto& out = filters[predefined_filter_type::FLYING_EXPLOSIVE];
		out.categoryBits = make_flags(C::FLYING_EXPLOSIVE);
		out.maskBits = standard_participation_except(C::LYING_ITEM, C::FLYING_EXPLOSIVE, C::CHARACTER, C::CHARACTER_WEAPON);
	}
	{
		auto& out = filters[predefined_filter_type::FLYING_MELEE];
		out.categoryBits = make_flags(C::FLYING_MELEE);
		out.maskBits = standard_participation_except(C::LYING_ITEM);
	}
	{

		auto& out = filters[predefined_filter_type::SHELL];
		out.categoryBits = make_flags(C::SHELL);
		out.maskBits = standard_participation_except(C::FLYING_BULLET, C::FLYING_BULLET, C::FLYING_EXPLOSIVE, C::FLYING_MELEE, C::CHARACTER_WEAPON);
	}
	{

		auto& out = filters[predefined_filter_type::PLANTED_EXPLOSIVE];
		out.categoryBits = make_flags(C::LYING_ITEM);
		out.maskBits = make_flags(C::QUERY, C::SHELL);
	}
	{

		auto& out = filters[predefined_filter_type::GLASS_OBSTACLE];
		out.categoryBits = make_flags(C::GLASS_OBSTACLE);
		out.maskBits = standard_participation();
	}

	{
		auto& out = filters[predefined_filter_type::PORTAL];
		out.categoryBits = make_flags(C::QUERY);

		/* This is just a default that will be overridden by editor_filter_flags anyway. */
		out.maskBits = standard_participation_except(C::CHARACTER_WEAPON, C::CAR_FLOOR);
	}
}

