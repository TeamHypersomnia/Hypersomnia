#include "physics_component.h"
#include <Box2D\Box2D.h>

namespace components {
	void physics::set_velocity(vec2 pixels) {
		body->SetLinearVelocity(pixels * PIXELS_TO_METERSf);
	}	
	
	void physics::set_linear_damping_vec(vec2 pixels) {
		body->SetLinearDampingVec(pixels * PIXELS_TO_METERSf);
	}

	void physics::apply_force(vec2 pixels) {
		body->ApplyForce(pixels * PIXELS_TO_METERSf, body->GetWorldCenter(), true);
	}

	float physics::get_mass() {
		return body->GetMass();
	}

	vec2 physics::velocity() {
		return vec2(body->GetLinearVelocity()) * METERS_TO_PIXELSf;
	}
}