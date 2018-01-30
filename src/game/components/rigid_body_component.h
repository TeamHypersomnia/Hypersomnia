#pragma once
#include "3rdparty/Box2D/Common/b2Math.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/templates/maybe_const.h"

#include "game/transcendental/component_synchronizer.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/debug_drawing_settings.h"

#include "augs/math/si_scaling.h"
#include "game/enums/rigid_body_type.h"

#include "game/detail/physics/damping_info.h"
#include "game/components/transform_component.h"

class relational_cache;

struct friction_connection {
	// GEN INTROSPECTOR struct friction_connection
	entity_id target;
	unsigned fixtures_connected = 0;
	// END GEN INTROSPECTOR
	friction_connection(entity_id t = entity_id()) : target(t) {}

	operator entity_id() const {
		return target;
	}
};

struct special_physics {
	// GEN INTROSPECTOR struct special_physics
	augs::stepped_cooldown dropped_or_created_cooldown;
	entity_id during_cooldown_ignore_collision_with;
	entity_id owner_friction_ground;
	augs::constant_size_vector<friction_connection, OWNER_FRICTION_GROUNDS_COUNT> owner_friction_grounds;
	// END GEN INTROSPECTOR

	//float measured_carried_mass = 0.f;
};

template <bool, class>
class basic_physics_mixin;

namespace components {
	struct rigid_body {
		static constexpr bool is_synchronized = true;

		rigid_body(
			const si_scaling = si_scaling(),
			const components::transform t = components::transform()
		);

		// GEN INTROSPECTOR struct components::rigid_body
		b2Transform transform;
		b2Sweep sweep;

		vec2 velocity;
		float angular_velocity = 0.f;

		special_physics special;

		// END GEN INTROSPECTOR

		void set_transform(
			const si_scaling,
			const components::transform&
		);
	};
}

namespace invariants {
	struct rigid_body {
		using implied_component = components::rigid_body;

		// GEN INTROSPECTOR struct invariants::rigid_body
		bool bullet = false;
		bool angled_damping = false;
		bool allow_sleep = true;

		rigid_body_type body_type = rigid_body_type::DYNAMIC;

		damping_info damping;
		// END GEN INTROSPECTOR
	};
};

class physics_world_cache;

template <class E>
class component_synchronizer<E, components::rigid_body> 
	: public synchronizer_base<E, components::rigid_body> 
{
	friend class ::physics_world_cache;

	auto& get_cache() const {
		return handle.get_cosmos().get_solvable_inferred({}).physics.get_rigid_body_cache(handle);
	}

	auto& body() const {
		return *get_cache().body.get(); 
	}

	template <class T>
	auto to_pixels(const T meters) const {
		return handle.get_cosmos().get_si().get_pixels(meters);
	}

	template <class T>
	auto to_meters(const T pixels) const {
		return handle.get_cosmos().get_si().get_meters(pixels);
	}

	using base = synchronizer_base<E, components::rigid_body>;
	using base::handle;

public:
	using base::synchronizer_base;
	using base::get_raw_component;

	void infer_caches() const;

	void set_velocity(const vec2) const;
	void set_angular_velocity(const float) const;

	void set_transform(const components::transform&) const;
	void set_transform(const entity_id) const;

	void apply_force(const vec2) const;
	void apply_force(const vec2, const vec2 center_offset, const bool wake = true) const;
	void apply_impulse(const vec2) const;
	void apply_impulse(const vec2, const vec2 center_offset, const bool wake = true) const;
	void apply_angular_impulse(const float) const;

	bool is_constructed() const;

	auto& get_special() const {
		return get_raw_component().special;
	}

	damping_info calculate_damping_info(const invariants::rigid_body&) const;

	vec2 get_velocity() const;
	float get_mass() const;
	float get_degrees() const;
	float get_radians() const;
	float get_degree_velocity() const;
	float get_radian_velocity() const;
	float get_inertia() const;
	vec2 get_position() const;
	components::transform get_transform() const;
	vec2 get_mass_position() const;
	vec2 get_world_center() const;
	bool test_point(const vec2) const;
};

template <class E>
float component_synchronizer<E, components::rigid_body>::get_mass() const {
	return body().GetMass();
}

template <class E>
float component_synchronizer<E, components::rigid_body>::get_degrees() const {
	return get_radians() * RAD_TO_DEG<float>;
}

template <class E>
float component_synchronizer<E, components::rigid_body>::get_radians() const {
	return body().GetAngle();
}

template <class E>
float component_synchronizer<E, components::rigid_body>::get_degree_velocity() const {
	return get_radian_velocity() * RAD_TO_DEG<float>;
}

template <class E>
float component_synchronizer<E, components::rigid_body>::get_radian_velocity() const {
	return body().GetAngularVelocity();
}

template <class E>
float component_synchronizer<E, components::rigid_body>::get_inertia() const {
	return body().GetInertia();
}

template <class E>
vec2 component_synchronizer<E, components::rigid_body>::get_position() const {
	return to_pixels(body().GetPosition());
}

template <class E>
components::transform component_synchronizer<E, components::rigid_body>::get_transform() const {
	return { get_position(), get_degrees() };
}

template <class E>
vec2 component_synchronizer<E, components::rigid_body>::get_mass_position() const {
	return to_pixels(body().GetWorldCenter());
}

template <class E>
vec2 component_synchronizer<E, components::rigid_body>::get_velocity() const {
	return to_pixels(body().GetLinearVelocity());
}

template <class E>
vec2 component_synchronizer<E, components::rigid_body>::get_world_center() const {
	return to_pixels(body().GetWorldCenter());
}

template <class E>
bool component_synchronizer<E, components::rigid_body>::test_point(const vec2 v) const {
	return body().TestPoint(b2Vec2(to_meters(v)));
}

template <class E>
bool component_synchronizer<E, components::rigid_body>::is_constructed() const {
	return get_cache().is_constructed();
}

template <class E>
damping_info component_synchronizer<E, components::rigid_body>::calculate_damping_info(const invariants::rigid_body& def) const {
	damping_info damping = def.damping;

	if (const auto* const movement = handle.template find<components::movement>()) {
		const auto& movement_def = handle.template get<invariants::movement>();

		const bool is_inert = movement->make_inert_for_ms > 0.f;

		if (is_inert) {
			damping.linear = 2;
		}
		else {
			damping.linear = movement_def.standard_linear_damping;
		}

		const auto requested_by_input = movement->get_force_requested_by_input(movement_def);

		if (requested_by_input.non_zero()) {
			if (movement->was_sprint_effective) {
				if (!is_inert) {
					damping.linear /= 4;
				}
			}
		}

		const bool make_inert = movement->make_inert_for_ms > 0.f;

		/* the player feels less like a physical projectile if we brake per-axis */
		if (!make_inert) {
			damping.linear_axis_aligned += vec2(
				requested_by_input.x_non_zero() ? 0.f : movement_def.braking_damping,
				requested_by_input.y_non_zero() ? 0.f : movement_def.braking_damping
			);
		}
	}

	return damping;
}

template <class E>
void component_synchronizer<E, components::rigid_body>::infer_caches() const {
	handle.get_cosmos().get_solvable_inferred({}).physics.infer_cache_for_rigid_body(handle);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_velocity(const vec2 pixels) const {
	auto& v = get_raw_component().velocity;
	v = to_meters(pixels);

	if (!is_constructed()) {
		return;
	}

	get_cache().body->SetLinearVelocity(b2Vec2(v));
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_angular_velocity(const float degrees) const {
	auto& v = get_raw_component().angular_velocity;
	v = DEG_TO_RAD<float> * degrees;

	if (!is_constructed()) {
		return;
	}

	get_cache().body->SetAngularVelocity(v);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_force(const vec2 pixels) const {
	apply_force(pixels, vec2(0, 0), true);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_force(
	const vec2 pixels, 
	const vec2 center_offset, 
	const bool wake
) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f)) {
		return;
	}

	const auto body = get_cache().body.get();
	auto& data = get_raw_component();

	const auto force = handle.get_cosmos().get_fixed_delta().in_seconds() * to_meters(pixels);
	const auto location = vec2(body->GetWorldCenter() + b2Vec2(to_meters(center_offset)));

	body->ApplyLinearImpulse(
		b2Vec2(force), 
		b2Vec2(location), 
		wake
	);

	data.angular_velocity = body->GetAngularVelocity();
	data.velocity = body->GetLinearVelocity();

	if (DEBUG_DRAWING.draw_forces && force.non_zero()) {
		/* 
			Warning: bodies like player's crosshair recoil might have their forces drawn 
			in the vicinity of (0, 0) coordinates instead of near wherever the player is.
		*/

		auto& lines = DEBUG_LOGIC_STEP_LINES;
		lines.emplace_back(green, to_pixels(location) + to_pixels(force), to_pixels(location));
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_impulse(const vec2 pixels) const {
	apply_impulse(pixels, vec2(0, 0), true);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_impulse(
	const vec2 pixels, 
	const vec2 center_offset, 
	const bool wake
) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f)) {
		return;
	}

	auto body = get_cache().body.get();
	auto& data = get_raw_component();

	const vec2 force = to_meters(pixels);
	const vec2 location = vec2(body->GetWorldCenter()) + to_meters(center_offset);

	body->ApplyLinearImpulse(b2Vec2(force), b2Vec2(location), true);
	data.angular_velocity = body->GetAngularVelocity();
	data.velocity = body->GetLinearVelocity();

	if (DEBUG_DRAWING.draw_forces && pixels.non_zero()) {
		/* 
			Warning: bodies like player's crosshair recoil might have their forces drawn 
			in the vicinity of (0, 0) coordinates instead of near wherever the player is.
		*/

		DEBUG_PERSISTENT_LINES.emplace_back(green, to_pixels(location) + pixels, to_pixels(location));
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_angular_impulse(const float imp) const {
	ensure(is_constructed());
	auto& body = *get_cache().body.get();
	auto& data = get_raw_component();

	body.ApplyAngularImpulse(imp, true);
	data.angular_velocity = body.GetAngularVelocity();
}


template <class E>
void component_synchronizer<E, components::rigid_body>::set_transform(const entity_id id) const {
	set_transform(handle.get_cosmos()[id].get_logic_transform());
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_transform(const components::transform& transform) const {
	auto& data = get_raw_component();

	data.set_transform(
		handle.get_cosmos().get_common_significant().si,
		transform
	);

	if (!is_constructed()) {
		return;
	}

	auto& body = *get_cache().body.get();

	if (!(body.m_xf == data.transform)) {
		body.m_xf = data.transform;
		body.m_sweep = data.sweep;

		auto* broadPhase = &body.m_world->m_contactManager.m_broadPhase;

		for (auto* f = body.m_fixtureList; f; f = f->m_next)
		{
			f->Synchronize(broadPhase, body.m_xf, body.m_xf);
		}
	}	
}
