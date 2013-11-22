#include "stdafx.h"
#include "steering_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/steering_message.h"
#include "../components/visibility_component.h"

#include "render_system.h"
#include "physics_system.h"
#include <iostream>

#include "utility/randval.h"

steering_system::obstacle_avoidance_input::obstacle_avoidance_input() 
	: visibility_edges(nullptr), discontinuities(nullptr), output(nullptr) {
}
steering_system::avoidance_input::avoidance_input() 
	: avoidance_rectangle_length(0.f), avoidance_rectangle_width(0.f), ignore_discontinuities_narrower_than(0.f) {
}
steering_system::containment_input::containment_input() 
	: ray_count(0), randomize_rays(false), only_threats_in_OBB(false), physics(nullptr), ray_filter(nullptr), ignore_entity(nullptr) {
}
steering_system::steering_input::steering_input() 
	: speed(0.f), distance(0.f), max_speed(0.f), radius_of_effect(0.f), directed(false) {
}

void steering_system::process_events(world& owner) {}
void steering_system::process_entities(world& owner) {}

vec2<> steering_system::seek(steering_input in) {
	/* pathological case, we don't need to push further */
	if (in.distance < 5.f) return vec2<>(0, 0);

	/* if we want to slowdown on arrival */
	if (in.radius_of_effect > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::min(in.max_speed, in.max_speed * (in.distance / in.radius_of_effect));
		/* obtain desired velocity, direction is normalized */
		auto desired_velocity = in.direction * clipped_speed;
		return desired_velocity - in.velocity;
	}

	/* steer in the direction of difference between maximum desired speed and the actual velocity
		note that the vector we substract from is MAXIMUM velocity so we effectively increase velocity up to max_speed
	*/
	return (in.direction * in.max_speed - in.velocity);
}

vec2<> steering_system::flee(steering_input in) {
	/* if we want to constrain effective fleeing range */
	if (in.radius_of_effect > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::max(0.f, (in.max_speed * (1 - (in.distance / in.radius_of_effect))));
		auto desired_velocity = in.direction * clipped_speed;
		
		if (desired_velocity.non_zero())
			return desired_velocity - in.velocity;
		else return vec2<>(0.f, 0.f);
	}

	return in.direction * in.max_speed - in.velocity;
}

vec2<> steering_system::predict_interception(steering_input in, vec2<> target_velocity, float max_prediction_ms,
	bool flee_prediction) {
	/* how parallel is our current velocity and the direction we our target is to */
	auto forwardness = in.unit_vel.dot(in.direction);
	/* how parallel is our current velocity and the target's current velocity */
	auto parallelness = in.unit_vel.dot(target_velocity / target_velocity.length());

	float time_factor = 1.f;
	
	/* if we are fleeing and our chaser is right behind our back, there's no point in predicting far positions 
		as it would turn as towards the chaser */
	if (flee_prediction && forwardness < -0.707f && parallelness > 0.707f)
		time_factor = 0.2f;
	/* if we are chasing the target and it is dead ahead running towards us, there's no point in predicting far positions
		as it would turn as away from the target
	*/
	else if (forwardness > 0.707f && parallelness < -0.707f)
		time_factor = 0.2f;

	/* return the point of interception, clamping maximum prediction time to max_prediction_ms milliseconds (velocities are in seconds) */
	return in.target + target_velocity * std::min(max_prediction_ms / 1000.f, time_factor * (in.distance / in.speed));
}

#include <queue>

render_system* _render = nullptr;

steering_system::avoidance_info_output steering_system::get_avoidance_info(obstacle_avoidance_input in) {
		avoidance_info_output out;
		float velocity_angle = in.unit_vel.get_degrees();
		
		/* copied vector to hold rotated vertices */
		auto rotated_verts = *in.shape_verts;
		
		/* rotate to velocity's frame of reference */
		for (auto& v : rotated_verts) {
			v.rotate(-velocity_angle, vec2<>());
		}

		int topmost =	 (&*std::min_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
		int bottommost = (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
		int rightmost =  (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.x < b.x; })) - rotated_verts.data();
		
		rotated_verts[topmost].y -= in.avoidance_rectangle_width;
		rotated_verts[bottommost].y += in.avoidance_rectangle_width;

		/* these vertices near position */
		out.avoidance[0] = in.position + vec2<>(rotated_verts[topmost]).rotate(velocity_angle, vec2<>());
		out.avoidance[3] = in.position + vec2<>(rotated_verts[bottommost]).rotate(velocity_angle, vec2<>());

		/* these vertices far away */
		out.avoidance[1] = vec2<>(rotated_verts[rightmost].x + in.avoidance_rectangle_length, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + in.position;
		out.avoidance[2] = vec2<>(rotated_verts[rightmost].x + in.avoidance_rectangle_length, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + in.position;

		/* rightmost line for avoidance info */
		out.rightmost_line[0] = vec2<>(rotated_verts[rightmost].x, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + in.position;
		out.rightmost_line[1] = vec2<>(rotated_verts[rightmost].x, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + in.position;

		/* create b2PolygonShape to check for intersections with edges */
		b2PolygonShape avoidance_rectangle_shape;
		b2Vec2 rect_copy[4];
		std::copy(out.avoidance, out.avoidance + 4, rect_copy);
		//std::reverse(rect_copy, rect_copy + 4);
		avoidance_rectangle_shape.Set(rect_copy, 4);

		/* visibility points are in clockwise order */
		if (in.visibility_edges) {
			for (size_t i = 0; i < in.visibility_edges->size(); ++i) {
				/* create b2EdgeShape representing the outer visibility triangle's edge */
				b2EdgeShape edge_shape;
				edge_shape.Set((*in.visibility_edges)[i].first, (*in.visibility_edges)[i].second);

				/* we don't need to transform edge or ray since they are in the same frame of reference
				but we have to prepare dummy b2Transform as an argument for b2TestOverlap
				*/
				b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

				/* if an edge intersects the avoidance rectangle */
				if (b2TestOverlap(&avoidance_rectangle_shape, 0, &edge_shape, 0, null_transform, null_transform)) {
					out.intersections.push_back(i);
				}
			}
		}

		return out;
}

vec2<> steering_system::containment(containment_input in) {
	if (in.speed < 0.1f) return vec2<>();
	
	if (in.directed) 
		in.unit_vel = in.direction;

	obstacle_avoidance_input obstacle_in;
	obstacle_in.avoidance_input::operator=(in);
	auto avoidance = get_avoidance_info(obstacle_in);

		if (_render->draw_avoidance_info) {
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[0], avoidance.avoidance[1], graphics::pixel_32(255, 255, 255, 255)));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[1], avoidance.avoidance[2], graphics::pixel_32(255, 255, 255, 255)));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[2], avoidance.avoidance[3], graphics::pixel_32(255, 255, 255, 255)));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[3], avoidance.avoidance[0], graphics::pixel_32(255, 255, 255, 255)));
			
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.rightmost_line[0], avoidance.rightmost_line[1], graphics::pixel_32(255, 255, 255, 255)));
		}

		vec2<> rayline = (avoidance.avoidance[3] - avoidance.avoidance[0]);
		float avoidance_width = rayline.length();
		float avoidance_length = (avoidance.avoidance[0] - avoidance.avoidance[1]).Length();
		rayline /= avoidance_width;

		vec2<> steering;
		for (int i = 0; i < in.ray_count; ++i) {
			vec2<> ray_location = avoidance.avoidance[0];
			if (in.randomize_rays)
				ray_location += rayline * avoidance_width * randval(0.f, 1.f);
			else
				ray_location += rayline * (avoidance_width / (in.ray_count-1)) * i;

			
			vec2<> p1 = ray_location, p2;

			if (in.only_threats_in_OBB)
				p2 = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);
			else
				p2 = ray_location.project_onto(avoidance.avoidance[1], avoidance.avoidance[2]);
			
			_render->manually_cleared_lines.push_back(render_system::debug_line(p1, p2, graphics::pixel_32(255, 0, 0, 255)));
			auto output = in.physics->ray_cast_px(p1, p2, in.ray_filter, in.ignore_entity);

			/* if our ray hits anything */
			if (output.hit) {
				_render->manually_cleared_lines.push_back(render_system::debug_line(p1, output.intersection, graphics::pixel_32(0, 255, 255, 255)));
				/* if it's closer than rightmost_projection the weight should be even bigger than 1.0 */

				vec2<> rightmost_projection = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);
				
				float proportion = (output.intersection - rightmost_projection).length_sq() / (avoidance_length*avoidance_length);
				steering += output.normal * (1 - proportion);
			}
		}

		return steering;
}

bool steering_system::avoid_collisions(obstacle_avoidance_input in) {
	if (in.speed < 0.001f) return false;
		/*
		first - distance from target
		second - vertex pointer
		*/
	if (in.directed)
		in.unit_vel = in.direction;

		struct navigation_candidate {
			vec2<> vertex_ptr;
			float linear_distance;
			float angular_distance;
			bool clockwise;

			bool operator < (const navigation_candidate& b) const {
				return angular_distance > b.angular_distance;
			}

			navigation_candidate(vec2<> v = vec2<>(), float a = 0.f, float d = 0.f, bool c = false) :
				vertex_ptr(v), angular_distance(a), linear_distance(d), clockwise(c) {};
		};

		//navigation_candidate best_navigation_candidate;
		std::priority_queue < navigation_candidate > candidates;

		/* shortcut */
		auto& visibility_edges = (*in.visibility_edges);

		auto check_navigation_candidate = [&candidates, &in](vec2<> v, bool cw) {
			auto v1 = v - in.position;
			float dist = v1.length();
			auto v2 = in.unit_vel;
			if(dist != 0.f) v1 /= dist;
			
			float angular_distance = v1.dot(v2);
			/* minus here because if its 1.0 it is totally parallel and we want the less the better */
			candidates.push(navigation_candidate(v, -angular_distance, dist, cw));
		};

		int edges_num = static_cast<int>(visibility_edges.size());

		auto wrap = [edges_num](int ix){
			if (ix < 0) return edges_num + ix;
			return ix % edges_num;
		};

		/* these vertices near position */
		if (in.avoidance_rectangle_length <= 0.0001f) return false;

		auto avoidance = get_avoidance_info(in);

		for(auto& i : avoidance.intersections) {
			/* navigate to the left end of the edge*/
			for (int j = i, k = 0; k < edges_num; j = wrap(j - 1), ++k)
				/* if left-handed vertex of candidate edge and right-handed vertex of previous edge do not match, we have a lateral obstacle vertex */
				if (!visibility_edges[j].first.compare(visibility_edges[wrap(j - 1)].second, in.ignore_discontinuities_narrower_than)) {

					bool such_discontinuity_exists = false;
					/* check if there exists any discontinuity with such an edge index and winding */
					for (auto& disc : *in.discontinuities) {
						if (disc.edge_index == j && disc.winding == disc.LEFT) {
							such_discontinuity_exists = true;
							break;
						}
					}

					if (such_discontinuity_exists) {
						check_navigation_candidate(visibility_edges[j].first, false);
						break;
					}
				}
				else {
					if (_render->draw_avoidance_info) {
						_render->manually_cleared_lines.push_back(render_system::debug_line(visibility_edges[j].first, visibility_edges[j].second, graphics::pixel_32(255, 0, 0, 255)));
						_render->manually_cleared_lines.push_back(render_system::debug_line(visibility_edges[wrap(j - 1)].first, visibility_edges[wrap(j - 1)].second, graphics::pixel_32(255, 0, 0, 255)));
					}
				}

				/* navigate to the right end of the edge */
				for (int j = i, k = 0; k < edges_num; j = wrap(j + 1), ++k)
					/* if right-handed vertex of candidate edge and left-handed vertex of next edge do not match, we have a lateral obstacle vertex */
					if (!visibility_edges[j].second.compare(visibility_edges[wrap(j + 1)].first, in.ignore_discontinuities_narrower_than)) {

						bool such_discontinuity_exists = false;
						/* check if there exists any discontinuity with such an edge index and winding */
						for (auto& disc : *in.discontinuities) {
							if (disc.edge_index == j && disc.winding == disc.RIGHT) {
								such_discontinuity_exists = true;
								break;
							}
						}

						if (such_discontinuity_exists) {
							check_navigation_candidate(visibility_edges[j].second, true);
							break;
						}
					}
					else {
						if (_render->draw_avoidance_info) {
							_render->manually_cleared_lines.push_back(render_system::debug_line(visibility_edges[j].first, visibility_edges[j].second, graphics::pixel_32(255, 0, 0, 255)));
							_render->manually_cleared_lines.push_back(render_system::debug_line(visibility_edges[wrap(j + 1)].first, visibility_edges[wrap(j + 1)].second, graphics::pixel_32(255, 0, 0, 255)));
						}
					}
		}

		auto draw_avoidance = [&avoidance](graphics::pixel_32 col) {
			if (_render->draw_avoidance_info) {
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[0], avoidance.avoidance[1], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[1], avoidance.avoidance[2], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[2], avoidance.avoidance[3], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[3], avoidance.avoidance[0], col));
			}
		};
		
		draw_avoidance(graphics::pixel_32());

		/* if we have specified a single candidate for navigation,
		which means there is an obstacle on the way */
		
		/* shortcut */
		auto& best_candidate = *in.output;

		while (!candidates.empty()) {
			/* save output */
			best_candidate = candidates.top().vertex_ptr;

			if (_render->draw_avoidance_info)
			_render->manually_cleared_lines.push_back(render_system::debug_line(in.position, best_candidate, graphics::pixel_32(0, 255, 0, 255)));

			float angle = (best_candidate - in.position).angle_between(in.unit_vel);

			for (auto& p : avoidance.avoidance)
				p = vec2<>(p).rotate(angle, in.position);
			
			if (_render->draw_avoidance_info)
				_render->manually_cleared_lines.push_back(render_system::debug_line(in.position, best_candidate, graphics::pixel_32(0, 255, 255, 255)));

			draw_avoidance(graphics::pixel_32());
			return true;
		}

		/* no obstacle on the way, no force applied */
		return false;
}

void steering_system::steering_input::set_target(vec2<> t) {
	target = t;

	direction = target - position;
	distance = direction.length();

	/* handle pathological case, if we're directly at target just choose random unit vector */
	if (distance == 0.f)
		direction = vec2<>(1, 0);
	else
		direction /= distance;
}

void steering_system::steering_input::set_velocity(vec2<> v) {
	velocity = v;
	unit_vel = v;
	speed = v.length();

	if (speed == 0.f)
		unit_vel = vec2<>(0.f, 0.f);
	else
		unit_vel /= speed;
}


void steering_system::substep(world& owner) {
	auto& render = owner.get_system<render_system>();
	auto& physics = owner.get_system<physics_system>();
	_render = &render;

	render.manually_cleared_lines.clear();

	for (auto it : targets) {
		auto& steering = it->get<components::steering>();
		auto& transform = it->get<components::transform>().current;
		auto body = it->get<components::physics>().body;

		/* extract ALL the vertices from the physics body, it will be then used by obstacle avoidance routines to calculate avoidance quad */
		std::vector<vec2<>> shape_verts;
		/* iterate through all the fixtures of a body */
		for (const b2Fixture* f = body->GetFixtureList(); f != nullptr; f = f->GetNext()) {
			/* we only support polygonal units */
			if (f->GetShape()->m_type != b2Shape::e_polygon) 
				throw std::runtime_error("only polygonal AI units are supported!");

			auto shape = reinterpret_cast<const b2PolygonShape*>(f->GetShape());

			for (int i = 0; i < shape->GetVertexCount(); ++i) 
				/* transform from Box2D coordinates */
				shape_verts.push_back(vec2<>(shape->GetVertex(i)).rotate(transform.rotation, vec2<>(0, 0)) * METERS_TO_PIXELSf);
		}

		vec2<> velocity = body->GetLinearVelocity(), resultant_force;
		velocity *= METERS_TO_PIXELSf;

		float max_speed = body->m_max_speed * METERS_TO_PIXELSf;

		auto draw_vector = [&transform, &render](vec2<> v, graphics::pixel_32 col){
			if (v.non_zero())
				render.manually_cleared_lines.push_back(render_system::debug_line(transform.pos, transform.pos + v, col));
		};
		
		/* iterate through all the requested behaviours */
		for (auto& ptr_behaviour : steering.active_behaviours) {
			using components::steering;
			vec2<> added_force;

			auto& behaviour = *ptr_behaviour;
			if (!behaviour.enabled) continue;

			/* prepare data that will be used by all steering behaviours */
			steering_input common_input;
			avoidance_input avoid_input;
			containment_input containment_input;
			obstacle_avoidance_input obstacle_avoidance_input;

			common_input.position = transform.pos;

			/* only for directed behaviours */
			if (behaviour.current_target) {
				common_input.set_target(behaviour.current_target->get<components::transform>().current.pos);
				common_input.directed = true;
			}

			common_input.set_velocity(velocity);
			common_input.max_speed = max_speed;
			common_input.radius_of_effect = behaviour.radius_of_effect;

			avoid_input.steering_input::operator=(common_input);
			avoid_input.avoidance_rectangle_width = behaviour.avoidance_rectangle_width;
			avoid_input.avoidance_rectangle_length = common_input.speed * behaviour.intervention_time_ms / 1000.f;

			/* clamp maximum intervention length*/
			if (behaviour.max_intervention_length > 1.f && avoid_input.avoidance_rectangle_length > behaviour.max_intervention_length)
				avoid_input.avoidance_rectangle_length = behaviour.max_intervention_length;

			avoid_input.shape_verts = &shape_verts;
			avoid_input.ignore_discontinuities_narrower_than = behaviour.ignore_discontinuities_narrower_than;

			containment_input.avoidance_input::operator=(avoid_input);
			containment_input.only_threats_in_OBB = behaviour.only_threats_in_OBB;
			containment_input.randomize_rays = behaviour.randomize_rays;
			containment_input.ray_count = behaviour.ray_count;
			containment_input.physics = &physics;
			containment_input.ray_filter = &it->get<components::visibility>().get_layer(behaviour.visibility_type).filter;
			containment_input.ignore_entity = it;

			/* undirected behaviours */
			if (behaviour.behaviour_type == steering::behaviour::CONTAINMENT) {
				added_force = containment(containment_input).normalize() * max_speed;
			}
			/* directed behaviours */
			else {
				if (behaviour.behaviour_type == steering::behaviour::FLEE) added_force = flee(common_input);
				if (behaviour.behaviour_type == steering::behaviour::SEEK) added_force = seek(common_input);
				if (
					behaviour.behaviour_type == steering::behaviour::PURSUIT ||
					behaviour.behaviour_type == steering::behaviour::EVASION
					) {
						common_input.set_target(behaviour.last_estimated_pursuit_position =
							predict_interception(common_input,
							vec2<>(behaviour.current_target->get<components::physics>().body->GetLinearVelocity())*METERS_TO_PIXELSf,
							behaviour.max_target_future_prediction_ms, behaviour.behaviour_type == steering::behaviour::EVASION));

						if (behaviour.behaviour_type == steering::behaviour::PURSUIT) added_force = seek(common_input);
						if (behaviour.behaviour_type == steering::behaviour::EVASION) added_force = flee(common_input);
				}
				if (behaviour.behaviour_type == steering::behaviour::OBSTACLE_AVOIDANCE) {
					obstacle_avoidance_input.avoidance_input::operator=(avoid_input);
					vec2<> navigate_to;
					auto& vision = it->get<components::visibility>().get_layer(behaviour.visibility_type);

					obstacle_avoidance_input.visibility_edges = &vision.edges;
					obstacle_avoidance_input.discontinuities = &vision.discontinuities;

					obstacle_avoidance_input.output = &navigate_to;

					if (avoid_collisions(obstacle_avoidance_input)) {
						common_input.set_target(navigate_to);
						containment_input.set_velocity(navigate_to - transform.pos);

						added_force = seek(common_input);
						added_force += containment(containment_input).normalize() * max_speed;
					}
				}
			}

			behaviour.last_output_force = added_force;
			added_force *= behaviour.weight;

			/* values less then 0.f indicate we don't want force clamping */
			if (behaviour.max_force_applied >= 0.f)
				added_force.clamp(behaviour.max_force_applied);
			
			if (render.draw_substeering_forces)
				draw_vector(added_force, behaviour.force_color);

			resultant_force += added_force;
		}

		/* values less then 0.f indicate we don't want force clamping */
		if (steering.max_resultant_force >= 0.f)
			resultant_force.clamp(steering.max_resultant_force);

		body->ApplyForce(resultant_force*PIXELS_TO_METERSf, body->GetWorldCenter());

		if (render.draw_steering_forces)
			draw_vector(resultant_force, graphics::pixel_32(255, 255, 255, 122));

		if (render.draw_velocities)
			draw_vector(velocity, graphics::pixel_32(0, 255, 0, 255));
	}
}

