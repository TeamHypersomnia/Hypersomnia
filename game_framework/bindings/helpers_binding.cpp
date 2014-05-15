#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../game/polygon_fader.h"

#include "../components/gun_component.h"
#include "../components/damage_component.h"
#include "../components/transform_component.h"

#include "../resources/render_info.h"
#include "entity_system/entity.h"
// sensibilia bottlenecks
void loop_instability_gun_bullets(pixel_32 init_color, std::vector<entity_ptr>* bullets, float instability,
	float timestep_multiplier, vec2<> base_gravity) {
	auto all_player_bullets = *bullets;
		all_player_bullets.erase(std::remove_if(std::begin(all_player_bullets), std::end(all_player_bullets), [&](entity_ptr& p){
			if (!p.exists()) return true;

			auto* ent = p.get();
			auto& dmg = ent->get<damage>();
			auto& model = ent->get<render>().model;
			auto& current = ent->get<transform>().current;

			dmg.max_lifetime_ms = (500 + 300 * instability) / timestep_multiplier;
			auto& body = *ent->get<physics>().body;

			auto vel = vec2<>(body.GetLinearVelocity());
			auto dist_from_start = dmg.lifetime.get<std::chrono::milliseconds>();
				
			auto dist_from_starting_point = (dmg.starting_point - current.pos).length();
			vel.set_length(0.005 * dist_from_start);
			vel += base_gravity / 10 * (dist_from_starting_point / 700);

			body.ApplyForce(vel, body.GetWorldCenter(), true);
			body.ApplyAngularImpulse(randval(0, 0.01), true);

			auto alpha_mult = (1 - (dist_from_start / dmg.max_lifetime_ms));
			set_polygon_color(model, pixel_32(init_color.r, init_color.g, init_color.b, 255));
		
			return false;
		}), std::end(all_player_bullets));
}




namespace bindings {
	luabind::scope _polygon_fader() {
		return
			luabind::def("loop_instability_gun_bullets", loop_instability_gun_bullets),

			luabind::class_<polygon_fader>("polygon_fader")
			.def(luabind::constructor<>())
			.def("add_trace", &polygon_fader::add_trace)
			.def("loop", &polygon_fader::loop)
			.def("generate_triangles", &polygon_fader::generate_triangles)
			.def("get_num_traces", &polygon_fader::get_num_traces)
			.def_readwrite("max_traces", &polygon_fader::max_traces)
			;
	}
}