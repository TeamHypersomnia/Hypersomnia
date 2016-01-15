#include "physics_component.h"
#include <Box2D\Box2D.h>

namespace components {
	void physics::set_velocity(vec2 pixels) {
		body->SetLinearVelocity(pixels * PIXELS_TO_METERSf);
	}	
	
	void physics::set_linear_damping(float damping) {
		body->SetLinearDamping(damping);
	}

	void physics::set_linear_damping_vec(vec2 pixels) {
		body->SetLinearDampingVec(pixels);
	}

	void physics::apply_force(vec2 pixels) {
		apply_force(pixels, vec2(0,0), true);
	}
	
	void physics::apply_force(vec2 pixels, vec2 center_offset, bool wake) {
		body->ApplyForce(pixels * PIXELS_TO_METERSf, body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf), wake);
	}


	float physics::get_mass() {
		return body->GetMass();
	}

	vec2 physics::get_position() {
		return METERS_TO_PIXELSf * body->GetPosition();
	}

	vec2 physics::velocity() {
		return vec2(body->GetLinearVelocity()) * METERS_TO_PIXELSf;
	}
}