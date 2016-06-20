#include "physics_component.h"

#include <Box2D\Box2D.h>

#include "graphics/renderer.h"
#include "fixtures_component.h"

#include "math/vec2.h"
#include "game/cosmos.h"
#include "game/systems/physics_system.h"
#include "ensure.h"

namespace components {
	physics& physics::operator=(const physics& p) {
		initialize_from_definition(p.get_definition());
	}

	physics::physics(const physics& p) {
		initialize_from_definition(p.get_definition());
	}

	physics::physics(const rigid_body_definition& def) {
		initialize_from_definition(def);
	}

	void physics::initialize_from_definition(const rigid_body_definition& def) {
		black = def;
		rigid_body_white_box::operator=(def);

		destroy_body();

		if (should_body_exist_now())
			build_body();
	}

	entity_id physics::get_entity() {
		return black_detail.body_owner;
	}

	void physics::build_body() {
		ensure(black_detail.body == nullptr);

		b2BodyDef def;
		def.type = b2BodyType(black.body_type);
		def.angle = 0;
		def.userData = get_entity();
		def.bullet = black.bullet;
		def.position = black.transform.pos * PIXELS_TO_METERSf;
		def.angle = black.transform.rotation * DEG_TO_RAD;
		def.angularDamping = black.angular_damping;
		def.linearDamping = black.linear_damping;
		def.fixedRotation = black.fixed_rotation;
		def.gravityScale = black.gravity_scale;
		def.active = true;
		def.linearVelocity = black.velocity * PIXELS_TO_METERSf;
		def.angularVelocity = black.angular_velocity * DEG_TO_RAD;

		black_detail.body = black_detail.parent_system->b2world.CreateBody(&def);
		black_detail.body->SetAngledDampingEnabled(black.angled_damping);

		const auto& all_fixture_entities = get_entity().get<components::physics>().black_detail.fixture_entities;

		for (auto fe : all_fixture_entities) {
			auto& fixtures = fe.get<components::fixtures>();

			if (fixtures.should_fixtures_exist_now()) {
				fixtures.destroy_fixtures();
				fixtures.build_fixtures();
			}
			else {
				fixtures.destroy_fixtures();
			}
		}
	}

	void physics::destroy_body() {
		if (black_detail.body != nullptr) {
			const auto& all_fixture_entities = get_entity().get<components::physics>().black_detail.fixture_entities;

			for (auto fe : all_fixture_entities)
				fe.get<components::fixtures>().destroy_fixtures();

			black_detail.parent_system->b2world.DestroyBody(black_detail.body);
		}
	}

	rigid_body_definition physics::get_definition() const {
		rigid_body_definition output;
		output.rigid_body_black_box::operator=(black);
		output.rigid_body_white_box::operator=(*this);
		return output;
	}

	void physics::set_body_type(type t) {
		black.body_type = t;

		destroy_body();

		if (should_body_exist_now())
			build_body();
	}

	void physics::set_activated(bool flag) {
		black.activated = flag;

		destroy_body();

		if (should_body_exist_now())
			build_body();
	}

	void physics::set_velocity(vec2 pixels) {
		black.velocity = pixels;

		if (!syncable_black_box_exists())
			return;

		black_detail.body->SetLinearVelocity(pixels * PIXELS_TO_METERSf);
	}	
	
	void physics::set_linear_damping(float damping) {
		black.linear_damping = damping;

		if (!syncable_black_box_exists())
			return;

		black_detail.body->SetLinearDamping(damping);
	}

	void physics::set_linear_damping_vec(vec2 damping) {
		black.linear_damping_vec = damping;

		if (!syncable_black_box_exists())
			return;

		black_detail.body->SetLinearDampingVec(damping);
	}

	void physics::apply_force(vec2 pixels) {
		apply_force(pixels, vec2(0,0), true);
	}
	
	void physics::apply_force(vec2 pixels, vec2 center_offset, bool wake) {
		ensure(syncable_black_box_exists());

		if (pixels.is_epsilon(2.f))
			return;

		vec2 force = pixels * PIXELS_TO_METERSf;
		vec2 location = black_detail.body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

		black_detail.body->ApplyForce(force, location, wake);

		if (renderer::get_current().debug_draw_forces && force.non_zero()) {
			auto& lines = renderer::get_current().logic_lines;
			lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
		}
	}

	void physics::apply_impulse(vec2 pixels) {
		apply_impulse(pixels, vec2(0, 0), true);
	}

	void physics::apply_impulse(vec2 pixels, vec2 center_offset, bool wake) {
		ensure(syncable_black_box_exists());

		if (pixels.is_epsilon(2.f))
			return;

		vec2 force = pixels * PIXELS_TO_METERSf;
		vec2 location = black_detail.body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

		black_detail.body->ApplyLinearImpulse(force, location, true);

		if (renderer::get_current().debug_draw_forces && force.non_zero()) {
			auto& lines = renderer::get_current().logic_lines;
			lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
		}
	}

	void physics::apply_angular_impulse(float imp) {
		ensure(syncable_black_box_exists());
		black_detail.body->ApplyAngularImpulse(imp, true);
	}

	float physics::get_mass() const {
		ensure(syncable_black_box_exists());
		return black_detail.body->GetMass();
	}

	float physics::get_angle() const {
		ensure(syncable_black_box_exists());
		return black_detail.body->GetAngle() * RAD_TO_DEG;
	}

	vec2 physics::get_position() const {
		ensure(syncable_black_box_exists());
		return METERS_TO_PIXELSf * black_detail.body->GetPosition();
	}

	vec2 physics::get_mass_position() const {
		ensure(syncable_black_box_exists());
		return METERS_TO_PIXELSf * black_detail.body->GetWorldCenter();
	}

	vec2 physics::velocity() const {
		ensure(syncable_black_box_exists());
		return vec2(black_detail.body->GetLinearVelocity()) * METERS_TO_PIXELSf;
	}

	vec2 physics::get_world_center() const {
		ensure(syncable_black_box_exists());
		return METERS_TO_PIXELSf * black_detail.body->GetWorldCenter();
	}

	vec2 physics::get_aabb_size() const {
		ensure(syncable_black_box_exists());
		b2AABB aabb;
		aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
		aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

		b2Fixture* fixture = black_detail.body->GetFixtureList();
		
		while (fixture != nullptr) {
			aabb.Combine(aabb, fixture->GetAABB(0));
			fixture = fixture->GetNext();
		}

		return vec2(aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y);
	}

	entity_id physics::get_owner_friction_ground() const {
		return owner_friction_ground;
	}
	
	void physics::set_transform(entity_id id) {
		set_transform(id.get<components::transform>());
	}

	void physics::set_transform(components::transform transform) {
		black.transform = transform;

		if (!syncable_black_box_exists())
			return;

		black_detail.body->SetTransform(transform.pos * PIXELS_TO_METERSf, transform.rotation * DEG_TO_RAD);
	}

	bool physics::should_body_exist_now() const {
		return black_detail.parent_system != nullptr && black.activated;
	}

	bool physics::syncable_black_box_exists() const {
		return black_detail.body != nullptr;
	}

	bool physics::test_point(vec2 v) const {
		return black_detail.body->TestPoint(v * PIXELS_TO_METERSf);
	}
}