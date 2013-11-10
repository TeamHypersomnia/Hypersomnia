#include "stdafx.h"
#include "steering_system.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/steering_message.h"
#include "../components/ai_component.h"

#include "render_system.h"
#include <iostream>

void steering_system::process_events(world& owner) {}
void steering_system::process_entities(world& owner) {}

vec2<> steering_system::seek(vec2<> position, vec2<> velocity, vec2<> target, float max_speed, float arrival_radius) {
	auto direction = target - position;
	auto distance = direction.length();
	direction /= distance;

	/* pathological case, we don't need to push further */
	if (distance < 2.f) return vec2<>(0, 0);

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

bool steering_system::avoid_collisions(vec2<> position, vec2<> velocity, float avoidance_rectangle_width, vec2<> target,
	std::vector < std::pair < vec2<>, vec2< >> > &visibility_edges, float intervention_time_ms, 
	vec2<>& best_candidate, components::steering::behaviour& info) {
		//float speed = velocity.length();
		//if (speed < 0.00001f) return false;
		//vec2<> direction = velocity / speed;


		//velocity.normalize();
		//if (!velocity.non_zero()) velocity = vec2<>(0, -1);
		//float speed = 500.f;
		//vec2<> direction = velocity;

		float speed = velocity.length();
		if (std::abs(velocity.x) < 0.01f && std::abs(velocity.y) < 0.01f) return false;
		vec2<> direction = velocity / speed;
		
		/* clockwise wound perpendicular direction */
		vec2<> perpendicular_direction = vec2<>(direction.y, -direction.x);

		float avoidance_rectangle_length = speed * intervention_time_ms / 1000.f;
		if (avoidance_rectangle_length <= 0.0001f) return false;

		std::vector<b2Vec2> avoidance_rectangle;
		avoidance_rectangle.resize(4);		
		/* these vertices near position */
		avoidance_rectangle[0] = position - perpendicular_direction *  avoidance_rectangle_width/2;
		avoidance_rectangle[3] = position + perpendicular_direction *  avoidance_rectangle_width/2;

		/* these vertices far away */
		avoidance_rectangle[1] = avoidance_rectangle[0] + direction * avoidance_rectangle_length;
		avoidance_rectangle[2] = avoidance_rectangle[3] + direction * avoidance_rectangle_length;

		auto draw_avoidance = [&avoidance_rectangle](graphics::pixel_32 col) {
			if (_render->draw_avoidance_info) {
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance_rectangle[0], avoidance_rectangle[1], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance_rectangle[1], avoidance_rectangle[2], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance_rectangle[2], avoidance_rectangle[3], col));
				_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance_rectangle[3], avoidance_rectangle[0], col));
			}
		};

		//draw_avoidance(graphics::pixel_32());
		
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

		auto check_navigation_candidate = [&target, &candidates, &position](vec2<>& v, bool cw) {
			float distance = (target - v).length();
				//+ (position - v).length();
			candidates.push(navigation_candidate(v, distance, cw));
			//if (!best_navigation_candidate.vertex_ptr ||
			//	best_navigation_candidate.distance > distance) best_navigation_candidate = navigation_candidate(&v, distance, cw);
		};

		int edges_num = static_cast<int>(visibility_edges.size());

		auto wrap = [edges_num](int ix){
			if (ix < 0) return edges_num + ix;
			return ix % edges_num;
		};

		/* create b2PolygonShape to check for intersections with edges */
		b2PolygonShape avoidance_rectangle_shape;
		auto rect_copy = avoidance_rectangle;
		std::reverse(rect_copy.begin(), rect_copy.begin() + 4);
		avoidance_rectangle_shape.Set(rect_copy.data(), 4);

		/* visibility points are in clockwise order */
		for (int i = 0; i < edges_num; ++i) {
			/* create b2EdgeShape representing the outer visibility triangle's edge */
			b2EdgeShape edge_shape;
			edge_shape.Set(visibility_edges[i].first, visibility_edges[i].second);

			/* we don't need to transform edge or ray since they are in the same space
			but we have to prepare dummy b2Transform as an argument for b2TestOverlap
			*/
			b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
			_render->manually_cleared_lines.push_back(render_system::debug_line(visibility_edges[i].first, visibility_edges[i].second, graphics::pixel_32(255, 255, 255, 255)));

			/* if an edge intersects the avoidance rectangle */
			if (b2TestOverlap(&avoidance_rectangle_shape, 0, &edge_shape, 0, null_transform, null_transform)) {

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
		}

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
			angle += offset;

			best_candidate.rotate(offset, position);

			//auto _avoidance_rect_copy = avoidance_rectangle;

			for (auto& p : avoidance_rectangle)
				p = vec2<>(p).rotate(angle, position);
			
			if (_render->draw_avoidance_info)
				_render->manually_cleared_lines.push_back(render_system::debug_line(position, best_candidate, graphics::pixel_32(0, 255, 255, 255)));

			//bool good_found = true;
			//
			//b2PolygonShape _avoidance_rectangle_shape;
			//std::reverse(_avoidance_rect_copy.begin(), _avoidance_rect_copy.begin() + 4);
			//_avoidance_rectangle_shape.Set(_avoidance_rect_copy.data(), 4);
			//
			//for (int i = 0; i < edges_num; ++i) {
			//	b2EdgeShape edge_shape;
			//	edge_shape.Set(visibility_edges[i].first, visibility_edges[i].second);
			//	b2Transform null_transform(b2Vec2(0.f, 0.f), b2Rot(0.f));
			//
			//	if (b2TestOverlap(&_avoidance_rectangle_shape, 0, &edge_shape, 0, null_transform, null_transform)) {
			//		good_found = false;
			//		break;
			//	}
			//}
			//
			//if (good_found) {
			//	avoidance_rectangle  = _avoidance_rect_copy;
			//	draw_avoidance(candidates.top().clockwise ? graphics::pixel_32(255, 0, 0, 255) : graphics::pixel_32(0
			//		, 0, 255, 255));
			//	return true;
			//}
			//
			//candidates.pop();

			//draw_avoidance(candidates.top().clockwise ? graphics::pixel_32(255, 0, 0, 255) : graphics::pixel_32(0
			//	, 0, 255, 255));
			draw_avoidance(graphics::pixel_32());
			return true;
		}

		/* no obstacle on the way, no force applied */
		return false;
}

void steering_system::substep(world& owner) {
	auto& render = owner.get_system<render_system>();
	_render = &render;

	render.manually_cleared_lines.clear();

	for (auto it : targets) {
		auto& steering = it->get<components::steering>();
		auto& transform = it->get<components::transform>().current;
		auto body = it->get<components::physics>().body;

		vec2<> velocity = body->GetLinearVelocity(), resultant_force;
		velocity *= METERS_TO_PIXELSf;

		float max_speed = body->m_max_speed * METERS_TO_PIXELSf;

		auto draw_vector = [&transform, &render](vec2<> v, graphics::pixel_32 col){
			if (v.non_zero())
				render.manually_cleared_lines.push_back(render_system::debug_line(transform.pos, transform.pos + v, col));
		};
		
		for (auto& ptr_behaviour : steering.active_behaviours) {
			using components::steering;
			vec2<> added_force;

			auto& behaviour = *ptr_behaviour;
			if (!behaviour.enabled || behaviour.current_target == nullptr) continue;

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
				auto& edges = it->get<components::ai>().
					get_visibility(components::ai::visibility::OBSTACLE_AVOIDANCE).edges;
				
				if (avoid_collisions(transform.pos, velocity,
					behaviour.avoidance_rectangle_width,
					target_transform.pos,
					edges,
					behaviour.intervention_time_ms, navigate_to, behaviour)) {
						/* there's an obstacle, seek it */
						added_force = seek(transform.pos, velocity, navigate_to,
							max_speed, behaviour.arrival_slowdown_radius);
						//added_force *= 0.f;
							//if ((added_force * 0.f).non_zero()) {
							//	//int no_elo = 2;
							//	//added_force = 0.f;
							//
							//	added_force = vec2<>(0.f, 0.f);
							//}
							//added_force = vec2<>(0.f, 0.f);
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

