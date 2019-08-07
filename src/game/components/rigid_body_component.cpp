#include "rigid_body_component.h"

#include "3rdparty/Box2D/Box2D.h"

#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "augs/ensure.h"
#include "game/debug_drawing_settings.h"

components::rigid_body::rigid_body(
	const si_scaling si,
	const transformr t
) {
	physics_transforms.set(si, t);
}

transformr physics_engine_transforms::get(const si_scaling si) const {
	return get().to_user_space(si);
}

transformr physics_engine_transforms::get() const {
	return { m_xf.p, m_sweep.a };
}

void physics_engine_transforms::set(
	const si_scaling si,
	const transformr& t
) {
	set(t.to_si_space(si));
}

void physics_engine_transforms::set(const transformr& t) {
	m_xf = t.operator b2Transform();

	m_sweep.localCenter.SetZero();
	m_sweep.c0 = m_xf.p;
	m_sweep.c = m_xf.p;
	m_sweep.a0 = t.rotation;
	m_sweep.a = t.rotation;
	m_sweep.alpha0 = 0.0f;
}

std::optional<b2AABB> find_aabb(const b2Body& body) {
	b2AABB aabb;

	aabb.lowerBound = b2Vec2(FLT_MAX,FLT_MAX);
	aabb.upperBound = b2Vec2(-FLT_MAX,-FLT_MAX); 

	const auto* fixture = body.GetFixtureList();
	
	if (!fixture) {
		return std::nullopt;
	}

	while (fixture != nullptr) {
		// TODO: handle child indices?

		aabb.Combine(aabb, fixture->GetAABB(0));
		fixture = fixture->GetNext();
	}

	return aabb;
}
