#pragma once
#include "3rdparty/Box2D/Box2D.h"

#include "augs/math/transform.h"
#include "game/cosmos/component_synchronizer.h"
#include "game/debug_drawing_settings.h"
#include "game/components/rigid_body_component.h"
#include "augs/templates/maybe_const.h"
#include "game/inferred_caches/find_physics_cache.h"

class physics_world_cache;
class physics_system;
class b2Body;
struct b2ContactEdge;

template <class E, class B>
void infer_damping(const E& handle, B& b);

template <class E>
class component_synchronizer<E, components::rigid_body> 
	: public synchronizer_base<E, components::rigid_body> 
{
	friend ::physics_world_cache;
	friend ::physics_system;

	auto find_cache() const {
		return find_rigid_body_cache(handle);
	}

	const maybe_const_ptr_t<is_handle_const_v<E>, b2Body> body_ptr;

	auto find_body_impl() const {	
		using T = decltype(find_cache()->body.get());

		if (auto cache = find_cache()) {
			return cache->body.get();
		}

		return T(nullptr);
	}

	auto find_body() const {	
		return body_ptr;
	}

	auto& body() const {
		auto maybe_body = find_body();
		ensure(maybe_body != nullptr);
		return *maybe_body;
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
	friend class portal_system;

public:
	using base::base;
	using base::get_raw_component;

	component_synchronizer(
		const typename base::component_pointer c, 
		const E& h
	) : 
		base(c, h),
		body_ptr(find_body_impl())
	{}

	void infer_caches() const;

	void infer_damping() const {
		if (const auto body = find_body()) {
			::infer_damping(handle, *body);
		}
	}

	void set_velocity(const vec2) const;
	void set_angular_velocity(const float) const;

	void set_radian_velocity(const float) const;

	void set_transform(const transformr&) const;
	
	template <class id_type>
	void set_transform(const id_type id) const {
		set_transform(handle.get_cosmos()[id].get_logic_transform());
	}

	void apply_force(const vec2) const;
	void apply_force(const vec2, const vec2 center_offset, const bool wake = true) const;
	void apply_torque(float) const;
	void apply_impulse(const vec2) const;
	void apply_impulse(const vec2, const vec2 center_offset, const bool wake = true, const float epsilon = 2.0f) const;

	void apply_angular_impulse(const float) const;

	void apply(impulse_input) const;

	void apply_linear(vec2 direction, impulse_amount_def impulse) const;
	void apply_angular(impulse_amount_def impulse) const;

	template <class body_type>
	void update_after_step(const body_type& b) const {
		auto& body = get_raw_component({});

		body.physics_transforms.m_xf = b.m_xf;
		body.physics_transforms.m_sweep = b.m_sweep;

		body.velocity = vec2(b.GetLinearVelocity());
		body.angular_velocity = b.GetAngularVelocity();
	}

	bool is_constructed() const;

	auto& get_special() const {
		return get_raw_component({}).special;
	}

	auto get_teleport_alpha() const {
		return get_special().get_teleport_alpha();
	}

	const b2ContactEdge* get_contact_list() const { 
		if (auto cache = find_cache()) {
			return cache->body.get()->GetContactList();
		}

		return nullptr;
	}

	vec2 get_velocity() const;
	float get_mass() const;
	float get_degrees() const;
	float get_radians() const;
	float get_degree_velocity() const;
	float get_radian_velocity() const;
	float get_inertia() const;
	vec2 get_position() const;
	transformr get_transform() const;
	vec2 get_mass_position() const;
	vec2 get_world_center() const;
	bool test_point(const vec2) const;

	std::optional<ltrb> find_aabb() const;

	void backup_velocities() const {
		get_special().saved_velocity = get_velocity();
		get_special().saved_angular_velocity = get_radian_velocity();
	}

	void restore_velocities() const {
		set_velocity(get_special().saved_velocity);
		set_radian_velocity(get_special().saved_angular_velocity);
	}
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
transformr component_synchronizer<E, components::rigid_body>::get_transform() const {
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
	return find_body() != nullptr;
}

template <class E>
void component_synchronizer<E, components::rigid_body>::infer_caches() const {
	handle.get_cosmos().get_solvable_inferred({}).physics.infer_rigid_body(handle);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_velocity(const vec2 pixels) const {
	auto& v = get_raw_component({}).velocity;
	v = to_meters(pixels);

	if (const auto body = find_body()) {
		body->SetLinearVelocity(b2Vec2(v));
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_angular_velocity(const float degrees) const {
	set_radian_velocity(degrees * DEG_TO_RAD<float>);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_radian_velocity(const float radians) const {
	auto& v = get_raw_component({}).angular_velocity;
	v = radians;

	if (const auto body = find_body()) {
		body->SetAngularVelocity(v);
	}
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
	apply_impulse(handle.get_cosmos().get_fixed_delta().in_seconds() * pixels, center_offset, wake, 0.0f);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_torque(
	const float amount
) const {
	apply_angular_impulse(handle.get_cosmos().get_fixed_delta().in_seconds() * amount);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply(const impulse_input impulse) const {
	apply_impulse(impulse.linear, vec2(0, 0), true);
	apply_angular_impulse(impulse.angular);
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_linear(const vec2 direction, const impulse_amount_def impulse) const {
	if (impulse.amount == 0.0f && impulse.mode != impulse_type::SET_VELOCITY) {
		return;
	}

	switch (impulse.mode) {
		case impulse_type::FORCE:
			apply_force(direction * impulse.amount);
			break;
		case impulse_type::IMPULSE:
			apply_impulse(direction * impulse.amount);
			break;
		case impulse_type::ADD_VELOCITY:
			set_velocity(get_velocity() + direction * impulse.amount);
			break;
		case impulse_type::SET_VELOCITY:
			set_velocity(direction * impulse.amount);
			break;

		default:
			break;
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_angular(const impulse_amount_def impulse) const {
	if (impulse.amount == 0.0f && impulse.mode != impulse_type::SET_VELOCITY) {	
		return;
	}

	switch (impulse.mode) {
		case impulse_type::FORCE:
			apply_torque(impulse.amount * DEG_TO_RAD<float>);
			break;
		case impulse_type::IMPULSE:
			apply_angular_impulse(impulse.amount * DEG_TO_RAD<float>);
			break;
		case impulse_type::ADD_VELOCITY:
			set_angular_velocity(get_degree_velocity() + impulse.amount);
			break;
		case impulse_type::SET_VELOCITY:
			set_angular_velocity(impulse.amount);
			break;

		default:
			break;
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
	const bool wake,
	const float epsilon
) const {
	if (epsilon != 0.0f && pixels.is_epsilon(epsilon)) {
		return;
	}

	if (const auto body = find_body()) {
		auto& data = get_raw_component({});

		const auto force = to_meters(pixels);
		const auto location = vec2(body->GetWorldCenter()) + to_meters(center_offset);

		body->ApplyLinearImpulse(
			b2Vec2(force), 
			b2Vec2(location), 
			wake
		);

		data.angular_velocity = body->GetAngularVelocity();
		data.velocity = body->GetLinearVelocity();

		if (DEBUG_DRAWING.draw_forces && pixels.is_nonzero()) {
			/* 
				Warning: bodies like player's crosshair recoil might have their forces drawn 
				in the vicinity of (0, 0) coordinates instead of near wherever the player is.
			*/

			auto& lines = DEBUG_LOGIC_STEP_LINES;
			lines.emplace_back(green, to_pixels(location) + pixels, to_pixels(location));
		}
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::apply_angular_impulse(const float imp) const {
	if (auto body = find_body()) {
		auto& data = get_raw_component({});

		body->ApplyAngularImpulse(imp, true);
		data.angular_velocity = body->GetAngularVelocity();
	}
}

template <class E>
void component_synchronizer<E, components::rigid_body>::set_transform(const transformr& transform) const {
	auto& data = get_raw_component({});

	data.physics_transforms.set(
		handle.get_cosmos().get_common_significant().si,
		transform
	);

	if (const auto body = find_body()) {
		if (!(body->m_xf == data.physics_transforms.m_xf)) {
			body->m_xf = data.physics_transforms.m_xf;
			body->m_sweep = data.physics_transforms.m_sweep;

			auto* broadPhase = &body->m_world->m_contactManager.m_broadPhase;

			for (auto* f = body->m_fixtureList; f; f = f->m_next)
			{
				f->Synchronize(broadPhase, body->m_xf, body->m_xf);
			}
		}
	}	
}

struct b2AABB;

std::optional<b2AABB> find_aabb(const b2Body& body);

template <class E>
std::optional<ltrb> component_synchronizer<E, components::rigid_body>::find_aabb() const {
	if (const auto body = find_body()) {
		if (const auto aabb = ::find_aabb(*body)) {
			const auto si = handle.get_cosmos().get_si();
			return ltrb(
				aabb->lowerBound.x,		
				aabb->lowerBound.y,		
				aabb->upperBound.x,		
				aabb->upperBound.x
			).to_user_space(si);
		}
	}

	return std::nullopt;
}
