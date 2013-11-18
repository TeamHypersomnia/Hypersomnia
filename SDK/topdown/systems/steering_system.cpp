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

void steering_system::process_events(world& owner) {}
void steering_system::process_entities(world& owner) {}

vec2<> steering_system::seek(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float arrival_radius) {
	auto direction = target - position;
	auto distance = direction.length();
	direction /= distance;

	/* pathological case, we don't need to push further */
	if (distance < 5.f) return vec2<>(0, 0);

	/* if we want to slowdown on arrival */
	if (arrival_radius > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::min(max_speed, max_speed * (distance / arrival_radius));
		/* obtain desired velocity, direction is normalized */
		auto desired_velocity = direction * clipped_speed;
		return desired_velocity - velocity;
	}

	/* steer in the direction of difference between maximum desired speed and the actual velocity
		note that the vector we substract from is MAXIMUM velocity so we effectively increase velocity up to max_speed
	*/
	return (direction * max_speed - velocity);
}

vec2<> steering_system::flee(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float flee_radius) {
	auto direction = position - target;
	auto distance = direction.length();

	/* handle pathological case, if we're directly at target just choose random unit vector */
	if (position == target)
		direction = vec2<>(1, 0);
	/* else just normalize offset */
	else direction /= distance;

	/* if we want to constrain effective fleeing range */
	if (flee_radius > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::max(0.f, (max_speed * (1 - (distance / flee_radius))));
		auto desired_velocity = direction * clipped_speed;
		
		if (desired_velocity.non_zero())
			return desired_velocity - velocity;
		else return vec2<>(0.f, 0.f);
	}

	return direction * max_speed - velocity;
}

vec2<> steering_system::predict_interception(vec2<> position, vec2<> velocity, vec2<> target, vec2<> target_velocity, float max_prediction_ms,
	bool flee_prediction) {

	auto offset = target - position;
	auto distance = offset.length();
	
	auto speed = velocity.length();
	auto unit_vel = velocity / speed;

	/* how parallel is our current velocity and the direction we our target is to */
	auto forwardness = unit_vel.dot(offset / distance);
	/* how parallel is our current velocity and the target's current velocity */
	auto parallelness = unit_vel.dot(target_velocity / target_velocity.length());

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
	return target + target_velocity * std::min(max_prediction_ms/1000.f, time_factor * (distance / speed));
}

#include <queue>

render_system* _render = nullptr;

struct avoidance_info {
	std::vector<int> intersections;
	vec2<> rightmost_line[2];
	b2Vec2 avoidance[4];
} get_avoidance_info(vec2<> position, float avoidance_rectangle_width, vec2<> direction, float avoidance_rectangle_length, steering_system::edges& visibility_edges,
	const std::vector<vec2<>>& shape_verts
	) {
		avoidance_info out;
		/* clockwise wound perpendicular direction */
		vec2<> perpendicular_direction = vec2<>(direction.y, -direction.x);

		float velocity_angle = direction.get_degrees();
		
		auto rotated_verts = shape_verts;
		/* rotate to velocity's frame of reference */
		for (auto& v : rotated_verts) {
			v.rotate(-velocity_angle, vec2<>());
		}

		int topmost =	 (&*std::min_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
		int bottommost = (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
		int rightmost =  (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.x < b.x; })) - rotated_verts.data();
		
		rotated_verts[topmost].y -= avoidance_rectangle_width;
		rotated_verts[bottommost].y += avoidance_rectangle_width;

		/* these vertices near position */
		out.avoidance[0] = position + vec2<>(rotated_verts[topmost]).rotate(velocity_angle, vec2<>());
		out.avoidance[3] = position + vec2<>(rotated_verts[bottommost]).rotate(velocity_angle, vec2<>());

		/* these vertices far away */
		out.avoidance[1] = vec2<>(rotated_verts[rightmost].x + avoidance_rectangle_length, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + position;
		out.avoidance[2] = vec2<>(rotated_verts[rightmost].x + avoidance_rectangle_length, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + position;

		/* rightmost line for avoidance info */
		out.rightmost_line[0] = vec2<>(rotated_verts[rightmost].x, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + position;
		out.rightmost_line[1] = vec2<>(rotated_verts[rightmost].x, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + position;

		/* create b2PolygonShape to check for intersections with edges */
		b2PolygonShape avoidance_rectangle_shape;
		b2Vec2 rect_copy[4];
		std::copy(out.avoidance, out.avoidance + 4, rect_copy);
		//std::reverse(rect_copy, rect_copy + 4);
		avoidance_rectangle_shape.Set(rect_copy, 4);

		/* visibility points are in clockwise order */
		for (size_t i = 0; i < visibility_edges.size(); ++i) {
			/* create b2EdgeShape representing the outer visibility triangle's edge */
			b2EdgeShape edge_shape;
			edge_shape.Set(visibility_edges[i].first, visibility_edges[i].second);

			/* we don't need to transform edge or ray since they are in the same frame of reference
			but we have to prepare dummy b2Transform as an argument for b2TestOverlap
			*/
			b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));

			/* if an edge intersects the avoidance rectangle */
			if (b2TestOverlap(&avoidance_rectangle_shape, 0, &edge_shape, 0, null_transform, null_transform)) {
				out.intersections.push_back(i);
			}
		}

		return out;
}

vec2<> steering_system::containment(vec2<> position, vec2<> velocity, float avoidance_rectangle_width, float avoidance_rectangle_length,
	int rays_count, bool random_distribution, physics_system& physics, const std::vector<augmentations::vec2<>>& shape_verts, b2Filter& ray_filter, 
	entity* ignore_entity, bool only_threats_in_OBB) {
		float speed = velocity.length();
		if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) return vec2<>(0, 0);
		vec2<> direction = velocity / speed;

		auto avoidance = get_avoidance_info(position, avoidance_rectangle_width, direction, avoidance_rectangle_length, edges(), shape_verts);

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
		for (int i = 0; i < rays_count; ++i) {
			vec2<> ray_location = avoidance.avoidance[0];
			if (random_distribution)
				ray_location += rayline * avoidance_width * randval(0.f, 1.f);
			else
				ray_location += rayline * (avoidance_width / (rays_count-1)) * i;

			
			vec2<> p1 = ray_location, p2;

			if (only_threats_in_OBB)
				p2 = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);
			else
				p2 = ray_location.project_onto(avoidance.avoidance[1], avoidance.avoidance[2]);
			
			_render->manually_cleared_lines.push_back(render_system::debug_line(p1, p2, graphics::pixel_32(255, 0, 0, 255)));
			auto output = physics.ray_cast_px(p1, p2, &ray_filter, ignore_entity);

			/* if our ray hits anything */
			if (output.hit) {
				_render->manually_cleared_lines.push_back(render_system::debug_line(p1, output.intersection, graphics::pixel_32(0, 255, 255, 255)));
				vec2<> rightmost_projection = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);
				/* if an obstacle is even closer than a rightmost line, which means it overlaps the object's OBB - steer away immediately */
				//if ((output.intersection - ray_location).length_sq() <
				//	(ray_location - rightmost_projection).length_sq()
				//	) {
				//		steering += (velocity * -1).normalize();
				//}
				///* otherwise the obstacle is beyond the rightmost line, weigh the normals depending on the distance of the intersection */
				//else {
					
					float proportion = (output.intersection - rightmost_projection).length_sq() / (avoidance_length*avoidance_length);
					steering += output.normal * (1 - proportion);
				//}
			}
		}

		steering.normalize();
		float paralellness = steering.dot(direction);
		vec2<> perpendiculars [] = { vec2<>(direction.y, -direction.x), vec2<>(-direction.y, direction.x) };
		
		if (perpendiculars[0].dot(steering) > perpendiculars[1].dot(steering)) {
			return perpendiculars[0];
		}
		else perpendiculars[1];

		//if (paralellness < -0.7f) {
		//	steering = perpendicular_direction;
		//}

		return steering;
}

bool steering_system::avoid_collisions(vec2<> position, vec2<> velocity, float avoidance_rectangle_width, vec2<> target,
	edges &visibility_edges, float intervention_time_ms, 
	vec2<>& best_candidate, components::steering::behaviour& info, const std::vector<vec2<>>& shape_verts) {
		float speed = velocity.length();
		if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) return false;
		vec2<> direction = velocity / speed;

		/*
		first - distance from target
		second - vertex pointer
		*/

		struct navigation_candidate {
			vec2<> vertex_ptr;
			float distance;
			bool clockwise;

			bool operator < (const navigation_candidate& b) const {
				return distance > b.distance;
			}

			navigation_candidate(vec2<> v = vec2<>(), float d = 0.f, bool c = false) :
				vertex_ptr(v), distance(d), clockwise(c) {};
		};

		//navigation_candidate best_navigation_candidate;
		std::priority_queue < navigation_candidate > candidates;

		auto check_navigation_candidate = [&candidates, &position, velocity](vec2<> v, bool cw) {
			auto v1 = v - position;
			auto v2 = velocity;
			v1.normalize();
			v2.normalize();
			float distance = v1.dot(v2);
			candidates.push(navigation_candidate(v, -distance, cw));
		};

		int edges_num = static_cast<int>(visibility_edges.size());

		auto wrap = [edges_num](int ix){
			if (ix < 0) return edges_num + ix;
			return ix % edges_num;
		};

		/* these vertices near position */
		float avoidance_rectangle_length = speed * intervention_time_ms / 1000.f;
		if (avoidance_rectangle_length <= 0.0001f) return false;

		auto avoidance = get_avoidance_info(position, avoidance_rectangle_width, direction, avoidance_rectangle_length, visibility_edges, shape_verts);

		for(auto& i : avoidance.intersections) {
			/* navigate to the left end of the edge*/
			for (int j = i, k = 0; k < edges_num; j = wrap(j - 1), ++k)
				/* if left-handed vertex of candidate edge and right-handed vertex of previous edge do not match, we have a lateral obstacle vertex */
				if (!visibility_edges[j].first.compare(visibility_edges[wrap(j - 1)].second, avoidance_rectangle_width)) {
					check_navigation_candidate(visibility_edges[j].first, false);
					break;
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
					if (!visibility_edges[j].second.compare(visibility_edges[wrap(j + 1)].first, avoidance_rectangle_width)) {

						check_navigation_candidate(visibility_edges[j].second, true);
						break;
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
		while (!candidates.empty()) {
			/* save output */
			best_candidate = candidates.top().vertex_ptr;

			if (info.last_decision_timer.get<std::chrono::milliseconds>() < info.decision_duration_ms) {
				if ((info.last_decision - best_candidate).length() > 10.f)
					best_candidate = info.last_decision;
				else info.last_decision = best_candidate;
			}
			else {
				info.last_decision_timer.reset();
				info.last_decision = best_candidate;
			}

			if (_render->draw_avoidance_info)
			_render->manually_cleared_lines.push_back(render_system::debug_line(position, best_candidate, graphics::pixel_32(0, 255, 0, 255)));

			float angle = (best_candidate - position).angle_between(direction);

			/* add offset to the angle so the edge of the avoidance rectangle only touches the vertex */
			float a = avoidance_rectangle_width / (2 * (best_candidate - position).length());
			
			if (a > 1) 
				a = 1;
			
			float offset = asin(a) / 0.01745329251994329576923690768489f;
			
			if (!candidates.top().clockwise)
				offset = -offset;
			//angle += offset;

			//best_candidate.rotate(offset, position);

			for (auto& p : avoidance.avoidance)
				p = vec2<>(p).rotate(angle, position);
			
			if (_render->draw_avoidance_info)
				_render->manually_cleared_lines.push_back(render_system::debug_line(position, best_candidate, graphics::pixel_32(0, 255, 255, 255)));

			draw_avoidance(graphics::pixel_32());
			return true;
		}

		/* no obstacle on the way, no force applied */
		return false;
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


			/* undirected behaviours */
			if (behaviour.behaviour_type == steering::behaviour::CONTAINMENT) {
				added_force = containment(transform.pos, velocity,
					behaviour.avoidance_rectangle_width, velocity.length() * behaviour.intervention_time_ms / 1000.f, behaviour.ray_count,
					behaviour.randomize_rays, physics, shape_verts,
					it->get<components::visibility>().get_layer(behaviour.visibility_type).filter,
					it, behaviour.only_threats_inside_OBB
					).normalize() * max_speed;
			}
			/* directed behaviours */
			else {
				auto& target_transform = behaviour.current_target->get<components::transform>().current;

				if (behaviour.behaviour_type == steering::behaviour::FLEE)
					added_force = flee(transform.pos, velocity, target_transform.pos, max_speed, behaviour.effective_fleeing_radius);
				if (behaviour.behaviour_type == steering::behaviour::SEEK)
					added_force = seek(transform.pos, velocity, target_transform.pos, max_speed, behaviour.arrival_slowdown_radius);
				if (
					behaviour.behaviour_type == steering::behaviour::PURSUIT ||
					behaviour.behaviour_type == steering::behaviour::EVASION
					) {

						behaviour.last_estimated_pursuit_position =
							predict_interception(transform.pos, velocity, target_transform.pos,
							vec2<>(behaviour.current_target->get<components::physics>().body->GetLinearVelocity())*METERS_TO_PIXELSf,
							behaviour.max_target_future_prediction_ms, behaviour.behaviour_type == steering::behaviour::EVASION);

						if (behaviour.behaviour_type == steering::behaviour::PURSUIT)
							added_force = seek(transform.pos, velocity, behaviour.last_estimated_pursuit_position, max_speed, behaviour.arrival_slowdown_radius);
						if (behaviour.behaviour_type == steering::behaviour::EVASION)
							added_force = flee(transform.pos, velocity, behaviour.last_estimated_pursuit_position, max_speed, behaviour.effective_fleeing_radius);
				}
				if (behaviour.behaviour_type == steering::behaviour::OBSTACLE_AVOIDANCE) {
					vec2<> navigate_to;
					auto& edges = it->get<components::visibility>().
						get_layer(behaviour.visibility_type).edges;

					if (avoid_collisions(transform.pos, velocity,
						behaviour.avoidance_rectangle_width,
						target_transform.pos,
						edges,
						behaviour.intervention_time_ms, navigate_to, behaviour, shape_verts)) {
							/* there's an obstacle, seek it */
							added_force = seek(transform.pos, velocity, navigate_to, max_speed, behaviour.arrival_slowdown_radius);

							added_force += containment(transform.pos, (navigate_to - transform.pos),
								behaviour.avoidance_rectangle_width, behaviour.intervention_time_ms, behaviour.ray_count,
								behaviour.randomize_rays, physics, shape_verts,
								it->get<components::visibility>().get_layer(behaviour.visibility_type).filter,
								it, behaviour.only_threats_inside_OBB
								).normalize() * max_speed;
					}
				}
			}

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

