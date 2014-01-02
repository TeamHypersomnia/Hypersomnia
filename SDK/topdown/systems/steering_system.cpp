#include "stdafx.h"
#include <iostream>

#include "utility/randval.h"

#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../messages/steering_message.h"
#include "../components/visibility_component.h"

#include "../game/body_helper.h"

#include "steering_system.h"
#include "render_system.h"
#include "physics_system.h"

using namespace components;

steering::scene::scene()
	: physics(nullptr), subject_entity(nullptr),
	vision(nullptr), shape_verts(nullptr), state(nullptr) {
}

steering::behaviour::behaviour() : max_force_applied(-1.f), weight(1.f) {}
steering::directed::directed() : radius_of_effect(-1.f), max_target_future_prediction_ms(-1.f)  {}
steering::avoidance::avoidance() 
	: avoidance_rectangle_width(0.f), intervention_time_ms(0.f), max_intervention_length(-1.f) {}
steering::wander::wander() : circle_radius(0.f), circle_distance(0.f), displacement_degrees(0.f) {}
steering::containment::containment() : ray_count(0), randomize_rays(false), only_threats_in_OBB(false) {}
steering::obstacle_avoidance::obstacle_avoidance() : ignore_discontinuities_narrower_than(1.f), navigation_correction(nullptr), navigation_seek(nullptr), visibility_type(0) {}
steering::flocking::flocking() : field_of_vision_degrees(360.f), square_side(0.f) {}

steering::object_info::object_info() : speed(0.f), max_speed(0.f) {}

void steering::object_info::set_velocity(vec2<> v) {
	velocity = v;

	/* update unit_vel and speed data accordingly */
	unit_vel = v;
	speed = v.length();

	/* pathological case, we don't want to divide by zero */
	if (speed == 0.f)
		unit_vel = vec2<>(0.f, 0.f);
	else
		unit_vel /= speed;
}

float steering::avoidance::get_avoidance_length(const object_info& subject) const {
	float avoidance_rectangle_length = subject.speed * intervention_time_ms / 1000.f;

	/* clamp maximum intervention length*/
	if (max_intervention_length > 1.f && avoidance_rectangle_length > max_intervention_length)
		avoidance_rectangle_length = max_intervention_length;

	return avoidance_rectangle_length;
}

steering::target_info::target_info() : is_set(false), distance(0.f) {}

void steering::target_info::set(vec2<> t, vec2<> vel) {
	is_set = true;
	info.position = t;
	info.set_velocity(vel);
}

void steering::target_info::set(const augmentations::entity_system::entity_ptr& t) {
	auto physics_comp = t.get()->find<physics>();
	
	if (physics_comp)
		set(t.get()->get<transform>().current.pos, vec2<>(physics_comp->body->GetLinearVelocity())*METERS_TO_PIXELSf);
	else 
		set(t.get()->get<transform>().current.pos);
}

void steering::target_info::calc_direction_distance(const object_info& in) {
	/* update direction and distance data accordingly */
	direction = info.position - in.position;
	distance = direction.length();

	/* handle pathological case, if we're directly at target just choose random unit vector */
	if (distance == 0.f)
		direction = vec2<>(1, 0);
	else
		direction /= distance;
}

render_system* _render = nullptr;

steering::avoidance::avoidance_info_output steering::avoidance::get_avoidance_info(const scene& in) {
	avoidance_info_output out;
	float velocity_angle = in.subject.unit_vel.get_degrees();
	float avoidance_rectangle_length = get_avoidance_length(in.subject);

	/* copied vector to hold rotated vertices */
	std::vector<vec2<>> rotated_verts;

	/* rotate to velocity's frame of reference */
	for (auto& v : *in.shape_verts) {
		rotated_verts.push_back(vec2<>(v - in.subject.position).rotate(-velocity_angle, vec2<>()));
	}

	int topmost = (&*std::min_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
	int bottommost = (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.y < b.y; })) - rotated_verts.data();
	int rightmost = (&*std::max_element(rotated_verts.begin(), rotated_verts.end(), [](vec2<> a, vec2<> b){ return a.x < b.x; })) - rotated_verts.data();

	rotated_verts[topmost].y -= avoidance_rectangle_width;
	rotated_verts[bottommost].y += avoidance_rectangle_width;

	/* these vertices near position */
	out.avoidance[0] = in.subject.position + vec2<>(rotated_verts[topmost]).rotate(velocity_angle, vec2<>());
	out.avoidance[3] = in.subject.position + vec2<>(rotated_verts[bottommost]).rotate(velocity_angle, vec2<>());

	/* these vertices far away */
	out.avoidance[1] = vec2<>(rotated_verts[rightmost].x + avoidance_rectangle_length, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + in.subject.position;
	out.avoidance[2] = vec2<>(rotated_verts[rightmost].x + avoidance_rectangle_length, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + in.subject.position;

	/* rightmost line for avoidance info */
	out.rightmost_line[0] = vec2<>(rotated_verts[rightmost].x, rotated_verts[topmost].y).rotate(velocity_angle, vec2<>()) + in.subject.position;
	out.rightmost_line[1] = vec2<>(rotated_verts[rightmost].x, rotated_verts[bottommost].y).rotate(velocity_angle, vec2<>()) + in.subject.position;

	return out;
}

std::vector<int> steering::avoidance::check_for_intersections(avoidance_info_output input, const std::vector<visibility::edge>& visibility_edges) {
	std::vector<int> intersections;
	
	/* create b2PolygonShape to check for intersections with edges */
	b2PolygonShape avoidance_rectangle_shape;
	b2Vec2 rect_copy[4];
	std::copy(input.avoidance, input.avoidance + 4, rect_copy);
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
			intersections.push_back(i);
		}
	}

	return intersections;
}

vec2<> steering::seek::seek_to(const object_info& subject, const target_info& target) const {
	/* pathological case, we don't need to push further */
	if (target.distance < 5.f) return vec2<>(0, 0);

	/* if we want to slowdown on arrival */
	if (radius_of_effect > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::min(subject.max_speed, subject.max_speed * (target.distance / radius_of_effect));
		/* obtain desired velocity, direction is normalized */
		auto desired_velocity = target.direction * clipped_speed;
		return desired_velocity - subject.velocity;
	}

	/* steer in the direction of difference between maximum desired speed and the actual velocity
	note that the vector we substract from is MAXIMUM velocity so we effectively increase velocity up to max_speed
	*/
	return (target.direction * subject.max_speed - subject.velocity);
}

vec2<> steering::seek::steer(scene in) {
	/* retrieve target data by copy */
	auto target = in.state->target;
	
	/* handle pursuit behaviour */
	if (max_target_future_prediction_ms > 0.f) 
		target.set(in.state->last_estimated_target_position = predict_interception(in.subject, target, false));

	return seek_to(in.subject, target);
}

vec2<> steering::flee::flee_from(const object_info& in, const target_info& target) const {
	/* if we want to constrain effective fleeing range */
	if (radius_of_effect > 0.f) {
		/* get the proportion and clip it to max_speed */
		auto clipped_speed = std::max(0.f, (in.max_speed * (1 - (target.distance / radius_of_effect))));
		auto desired_velocity = target.direction * clipped_speed;

		if (desired_velocity.non_zero())
			return desired_velocity - in.velocity;
		else return vec2<>(0.f, 0.f);
	}

	return (target.direction * (-1)) * in.max_speed - in.velocity;
}

vec2<> steering::flee::steer(scene in) {
	/* retrieve target data by copy */
	auto target = in.state->target;

	/* handle pursuit behaviour */
	if (max_target_future_prediction_ms > 0.f)
		target.set(in.state->last_estimated_target_position = predict_interception(in.subject, target, true));

	return flee_from(in.subject, target);
}


vec2<> steering::directed::predict_interception(const object_info& subject, const target_info& target, bool flee_prediction) {
	if (target.info.speed < std::numeric_limits<float>::epsilon() || subject.speed < std::numeric_limits<float>::epsilon())
		return target.info.position;
	
	/* obtain target's velocity */
	/* how parallel is our current velocity and the direction we our target is to */
	auto forwardness = subject.unit_vel.dot(target.direction);
	/* how parallel is our current velocity and the target's current velocity */
	auto parallelness = subject.unit_vel.dot(target.info.unit_vel);

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
	return target.info.position + target.info.velocity * std::min(max_target_future_prediction_ms / 1000.f, time_factor * (target.distance / subject.speed));
}

void steering::avoidance::optional_align(scene& in) {
	/* if we have specified a target for avoidance, we want the avoidance rectangle to face the target */
	if (in.state->target.is_set) 
		in.subject.unit_vel = in.state->target.direction;
}

vec2<> steering::containment::steer(scene in) {
	/* speed is too small to make any significant changes, return */
	if (in.subject.speed < 0.001f) return vec2<>();
	optional_align(in);

	/* get avoidance info */
	auto avoidance = get_avoidance_info(in);

	/* debug drawing */
	if (_render->draw_avoidance_info) {
		_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[0], avoidance.avoidance[1], graphics::pixel_32(255, 255, 255, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[1], avoidance.avoidance[2], graphics::pixel_32(255, 255, 255, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[2], avoidance.avoidance[3], graphics::pixel_32(255, 255, 255, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[3], avoidance.avoidance[0], graphics::pixel_32(255, 255, 255, 255)));

		_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.rightmost_line[0], avoidance.rightmost_line[1], graphics::pixel_32(255, 255, 255, 255)));
	}

	/* prepare the line we will cast rays along */
	vec2<> rayline = (avoidance.avoidance[3] - avoidance.avoidance[0]);
	/* how long is this line */
	float avoidance_width = rayline.length();
	/* how long will the cast rays be */
	float avoidance_length = (avoidance.avoidance[0] - avoidance.avoidance[1]).Length();
	/* normalize the ray line */
	rayline /= avoidance_width;

	/* resultant vector */
	vec2<> steering;

	for (int i = 0; i < ray_count; ++i) {
		/* this is where the ray line emerges from */
		vec2<> ray_location = avoidance.avoidance[0];

		if (randomize_rays)
			ray_location += rayline * avoidance_width * randval(0.f, 1.f);
		else
			ray_location += rayline * (avoidance_width / (ray_count - 1)) * i;


		/* prepare the ray to be cast */
		vec2<> p1 = ray_location, p2;

		/* if we are only interested in threats between leftmost and rightmost line */
		if (only_threats_in_OBB)
			p2 = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);
		else
			p2 = ray_location.project_onto(avoidance.avoidance[1], avoidance.avoidance[2]);

		if (_render->draw_avoidance_info)
			_render->manually_cleared_lines.push_back(render_system::debug_line(p1, p2, graphics::pixel_32(255, 0, 0, 255)));
		
		/* cast the ray and save output */
		auto output = in.physics->ray_cast_px(p1, p2, ray_filter, in.subject_entity);

		/* if our ray hits anything */
		if (output.hit) {
			
			if (_render->draw_avoidance_info)
				_render->manually_cleared_lines.push_back(render_system::debug_line(p1, output.intersection, graphics::pixel_32(0, 255, 255, 255)));
			
			vec2<> rightmost_projection = ray_location.project_onto(avoidance.rightmost_line[0], avoidance.rightmost_line[1]);

			/* if it's closer than rightmost_projection the weight should be even bigger than 1.0 */
			float proportion = (output.intersection - rightmost_projection).length_sq() / (avoidance_length*avoidance_length);
			
			/* add steering force with reference to normal at the intersection point */
			steering += output.normal * (1 - proportion);
		}
	}

	if (steering.non_zero()) {
		steering.normalize();

		vec2<> perpendicular_cw = in.subject.unit_vel.perpendicular_cw();
		vec2<> perpendicular_ccw = -perpendicular_cw;

		if (perpendicular_cw.dot(steering) > perpendicular_ccw.dot(steering)) {
			/* CW is more parallel */
			steering = perpendicular_cw * in.subject.max_speed;
		}
		else steering = perpendicular_ccw * in.subject.max_speed;
	}

	return steering;

	//steering *= in.subject.max_speed;
	//return steering;
}

vec2<> steering::obstacle_avoidance::steer(scene in) {
	float avoidance_rectangle_length = get_avoidance_length(in.subject);

	if (in.subject.speed < 0.001f) return vec2<>();
	/*
	first - distance from target
	second - vertex pointer
	*/
	optional_align(in);

	struct navigation_candidate {
		vec2<> vertex_ptr;
		float linear_distance;
		float angular_distance;
		bool clockwise;

		bool operator < (const navigation_candidate& b) const {
			return angular_distance < b.angular_distance;
		}

		navigation_candidate(vec2<> v = vec2<>(), float a = 0.f, float d = 0.f, bool c = false) :
			vertex_ptr(v), angular_distance(a), linear_distance(d), clockwise(c) {};
	};

	std::vector<navigation_candidate> candidates;

	/* shortcuts */
	auto& visibility_edges = in.vision->get_layer(visibility_type).edges;
	auto& discontinuities = in.vision->get_layer(visibility_type).discontinuities;

	auto check_navigation_candidate = [&candidates, &in](vec2<> v, bool cw) {
		/* vector pointing from entity to navigation candidate */
		auto to_navpoint = v - in.subject.position;
		float distance_to_navpoint = to_navpoint.length();
		if (distance_to_navpoint != 0.f) to_navpoint /= distance_to_navpoint;

		/* angle it takes to turn the velocity so it faces the navpoint */
		float angular_distance = to_navpoint.dot(in.subject.unit_vel);
		/* minus here because if its 1.0 it is totally parallel and we want the less the better */
		candidates.push_back(navigation_candidate(v, -angular_distance, distance_to_navpoint, cw));
	};

	int edges_num = static_cast<int>(visibility_edges.size());

	/* helper wrapping lambda for iteration */
	auto wrap = [edges_num](int ix){
		if (ix < 0) return edges_num + ix;
		return ix % edges_num;
	};

	if (avoidance_rectangle_length <= 1.f) return vec2<>();

	auto avoidance = get_avoidance_info(in);

	std::vector<int> intersections;

	if (in.vision) 
		intersections = check_for_intersections(avoidance, in.vision->get_layer(visibility_type).edges);

	for (auto& i : intersections) {
		/* navigate to the left end of the edge*/
		for (int j = i, k = 0; k < edges_num; j = wrap(j - 1), ++k)
			/* if left-handed vertex of candidate edge and right-handed vertex of previous edge do not match, we have a lateral obstacle vertex */
			if (!visibility_edges[j].first.compare(visibility_edges[wrap(j - 1)].second, ignore_discontinuities_narrower_than)) {

				/* check if there exists any discontinuity with such an edge index and winding,
				if there does not, it means our discontinuity is too narrow and we don't want to go there
				*/
				bool such_discontinuity_exists = false;
				for (auto& disc : discontinuities) {
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
				if (!visibility_edges[j].second.compare(visibility_edges[wrap(j + 1)].first, ignore_discontinuities_narrower_than)) {

					/* rest is analogous */
					bool such_discontinuity_exists = false;
					for (auto& disc : discontinuities) {
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

	auto draw_avoidance = [&avoidance, this](graphics::pixel_32 col) {
		if (_render->draw_avoidance_info) {
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[0], avoidance.avoidance[1], force_color));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[1], avoidance.avoidance[2], force_color));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[2], avoidance.avoidance[3], force_color));
			_render->manually_cleared_lines.push_back(render_system::debug_line(avoidance.avoidance[3], avoidance.avoidance[0], force_color));
		}
	};

	draw_avoidance(graphics::pixel_32());

	/* if we have specified a single candidate for navigation,
	which means there is an obstacle on the way */

	while (!candidates.empty()) {
		/* prepare best candidate for navigation */
		vec2<> best_candidate;

		auto& discontinuity_info = (*std::min_element(candidates.begin(), candidates.end()));
		/* get the best candidate for navigation */
		best_candidate = discontinuity_info.vertex_ptr;

		/* debug drawing */
		if (_render->draw_avoidance_info) {
			_render->manually_cleared_lines.push_back(render_system::debug_line(in.subject.position, best_candidate, graphics::pixel_32(0, 255, 0, 255)));

			/* angle the entity has to turn by to face the navigation candidate */
			float angle = (best_candidate - in.subject.position).angle_between(in.subject.unit_vel);

			for (auto& p : avoidance.avoidance)
				p = vec2<>(p).rotate(angle, in.subject.position);

			_render->manually_cleared_lines.push_back(render_system::debug_line(in.subject.position, best_candidate, graphics::pixel_32(0, 255, 255, 255)));

			draw_avoidance(graphics::pixel_32());
		}

		behaviour_state new_state(navigation_correction);
		new_state.target.set(best_candidate);
		new_state.target.calc_direction_distance(in.subject);

		target_info navigation_target;
		navigation_target.set(best_candidate);
		navigation_target.calc_direction_distance(in.subject);

		in.state = &new_state;

		vec2<> steering;

		if (navigation_seek)
			steering += navigation_seek->seek_to(in.subject, navigation_target) * navigation_seek->weight;
		if (navigation_correction) {
			vec2<> correction = 
				navigation_correction->steer(in);
			
			if (correction.non_zero()) {
				vec2<> perpendicular_cw = (best_candidate - in.subject.position).normalize().perpendicular_cw() * in.subject.max_speed;

				correction = discontinuity_info.clockwise ? perpendicular_cw : -perpendicular_cw;
			}

			steering += correction * navigation_correction->weight;
		}

		return steering.set_length(in.subject.velocity.length());
	}

	/* no obstacle on the way, no force applied */
	return vec2<>();
}

vec2<> steering::wander::steer(scene in) {
	/* rotate the current displacement angle by a random offset */
	in.state->current_wander_angle += randval(-displacement_degrees, displacement_degrees);

	/* these are self-explanatory */
	vec2<> circle_center = in.subject.position + circle_distance * in.subject.unit_vel;
	vec2<> displacement_position = circle_center + vec2<>(1, 0) * circle_radius;
	displacement_position.rotate(in.state->current_wander_angle, circle_center);

	vec2<> displacement_force = displacement_position - in.subject.position;

	if (_render->draw_wandering_info) {
		_render->manually_cleared_lines.push_back(render_system::debug_line(circle_center - vec2<>(circle_radius, 0), circle_center + vec2<>(circle_radius, 0), graphics::pixel_32(0, 255, 255, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(circle_center - vec2<>(0, circle_radius), circle_center + vec2<>(0, circle_radius), graphics::pixel_32(0, 255, 255, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(circle_center, displacement_position, graphics::pixel_32(255, 0, 0, 255)));
		_render->manually_cleared_lines.push_back(render_system::debug_line(in.subject.position, displacement_position, graphics::pixel_32(0, 255, 0, 255)));
	}

	return displacement_force.normalize() * in.subject.max_speed;
}

vec2<> steering::separation::steer(scene in) {
	auto bodies = in.physics->query_square_px(in.subject.position, square_side, &group, in.subject_entity).bodies;

	/* debug drawing */
	if (_render->draw_avoidance_info) {
		b2AABB aabb;
		aabb.lowerBound = in.subject.position - square_side / 2;
		aabb.upperBound = in.subject.position + square_side / 2;
		
		b2Vec2 whole_vision[] = {
			aabb.lowerBound,
			aabb.lowerBound + vec2<>(square_side, 0),
			aabb.upperBound,
			aabb.upperBound - vec2<>(square_side, 0)
		};

		_render->manually_cleared_lines.push_back(render_system::debug_line((vec2<>(whole_vision[0]) + vec2<>(-1.f, 0.f)), (vec2<>(whole_vision[1]) + vec2<>(1.f, 0.f))));
		_render->manually_cleared_lines.push_back(render_system::debug_line((vec2<>(whole_vision[1]) + vec2<>(0.f, -1.f)), (vec2<>(whole_vision[2]) + vec2<>(0.f, 1.f))));
		_render->manually_cleared_lines.push_back(render_system::debug_line((vec2<>(whole_vision[2]) + vec2<>(1.f, 0.f)), (vec2<>(whole_vision[3]) + vec2<>(-1.f, 0.f))));
		_render->manually_cleared_lines.push_back(render_system::debug_line((vec2<>(whole_vision[3]) + vec2<>(0.f, 1.f)), (vec2<>(whole_vision[0]) + vec2<>(0.f, -1.f))));
	}

	vec2<> center;
	int body_count = 0;
	for (auto& body : bodies) {
		vec2<> direction = ((vec2<>(body->GetPosition()) * METERS_TO_PIXELSf) - in.subject.position);
		float distance = direction.length();

		/* transform the parallellness to 0.f - 1.f space (0.f - parallel, 1.f - anti-parallel) */
		float parallellness = 1.f - (((direction / distance).dot(in.subject.unit_vel) + 1.f) / 2.f);

		/* calculate fov parallellness threshold */
		float threshold = (field_of_vision_degrees / 2.f) / 180.f;

		/* if the body's position is within the field of view, add to the resultant */
		if (parallellness <= threshold) {
			center += vec2<>(body->GetPosition()) * METERS_TO_PIXELSf;
			++body_count;
		}
	}

	if (body_count > 0) {
		center /= body_count;

		flee my_flee;
		target_info center_average;
		center_average.set(center);
		center_average.calc_direction_distance(in.subject);

		return my_flee.flee_from(in.subject, center_average).normalize() * in.subject.max_speed;
	}
	else {
		return vec2<>();
	}
}

void steering::behaviour_state::update_target_info(const object_info& subject) {
	if (target_from) {
		target.set(target_from);
	}
	
	/* calculate target information for all the behaviours */
	target.calc_direction_distance(subject);
}


void steering_system::process_events(world& owner) {}
void steering_system::process_entities(world& owner) { substep(owner);  }
void steering_system::substep(world& owner) {
	auto& render = owner.get_system<render_system>();
	auto& physics_sys = owner.get_system<physics_system>();
	_render = &render;

	render.manually_cleared_lines.clear();

	for (auto it : targets) {
		auto& steer = it->get<steering>();
		auto& position = it->get<transform>().current.pos;
		auto body = it->get<physics>().body;

		/* extract ALL the vertices from the physics body, it will be then used by obstacle avoidance routines to calculate avoidance quad,
			false means we want pixels
		*/
		auto shape_verts = topdown::get_transformed_shape_verts(*it, false);
		auto draw_vector = [&position, &render](vec2<> v, graphics::pixel_32 col){
			if (v.non_zero())
				render.manually_cleared_lines.push_back(render_system::debug_line(position, position + v, col));
		};
		
		/* resultant force that sums all behaviours and that is to be finally applied */
		vec2<> resultant_force;

		/* prepare data that will be used by all steering behaviours */
		steering::scene frame_of_reference;
		frame_of_reference.subject.position = position;
		frame_of_reference.subject.set_velocity(METERS_TO_PIXELSf * body->GetLinearVelocity());
		frame_of_reference.subject.max_speed = body->m_max_speed * METERS_TO_PIXELSf;
		frame_of_reference.shape_verts = &shape_verts;
		frame_of_reference.physics = &physics_sys;
		frame_of_reference.subject_entity = it;
		frame_of_reference.vision = it->find<visibility>();
		
		/* iterate through all the requested behaviours */
		for (auto ptr_behaviour : steer.active_behaviours) {
			vec2<> added_force;

			auto& behaviour = *ptr_behaviour;
			if (!behaviour.enabled) continue;

			frame_of_reference.state = &behaviour;
			frame_of_reference.state->update_target_info(frame_of_reference.subject);

			auto& subject_behaviour = *behaviour.subject_behaviour;

			added_force = subject_behaviour.steer(frame_of_reference);

			behaviour.last_output_force = added_force;
			added_force *= subject_behaviour.weight * behaviour.weight_multiplier;

			/* values less then 0.f indicate we don't want force clamping */
			if (subject_behaviour.max_force_applied >= 0.f)
				added_force.clamp(subject_behaviour.max_force_applied);

			if (render.draw_substeering_forces)
				draw_vector(added_force, subject_behaviour.force_color);

			resultant_force += added_force;
		}

		/* values less then 0.f indicate we don't want force clamping */
		if (steer.max_resultant_force >= 0.f)
			resultant_force.clamp(steer.max_resultant_force);

		body->ApplyForce(resultant_force*PIXELS_TO_METERSf, body->GetWorldCenter());

		if (render.draw_steering_forces)
			draw_vector(resultant_force, graphics::pixel_32(255, 255, 255, 122));

		if (render.draw_velocities)
			draw_vector(METERS_TO_PIXELSf * body->GetLinearVelocity(), graphics::pixel_32(0, 255, 0, 255));
	}
}

