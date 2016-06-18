#pragma once
#include "stdafx.h"
#include "bindings.h"
#include "bind_callbacks.h"
#include <luabind/iterator_policy.hpp>

#include "window_framework/event.h"

#include "../all_component_includes.h"
#include "../resources/particle_emitter_info.h"

#include "../systems/input_system.h"
#include "../systems/render_system.h"
#include "../systems/visibility_system.h"

using namespace components;

void set_density(b2Body* body, float density) {
	for (b2Fixture* it = body->GetFixtureList(); it; it = it->GetNext()) {
		it->SetDensity(density);
	}

	body->ResetMassData();
}

void set_friction(b2Body* body, float friction) {
	for (b2Fixture* it = body->GetFixtureList(); it; it = it->GetNext()) {
		it->SetFriction(friction);
	}

	body->ResetMassData();
}

void set_restitution(b2Body* body, float restitution) {
	for (b2Fixture* it = body->GetFixtureList(); it; it = it->GetNext()) {
		it->SetRestitution(restitution);
	}

	body->ResetMassData();
}

void set_filter(b2Body* body, b2Filter filter) {
	for (b2Fixture* it = body->GetFixtureList(); it; it = it->GetNext()) {
		it->SetFilterData(filter);
	}

	body->ResetMassData();
}

struct dummy_Box2D {

};

namespace bindings {
	struct dummy_mouse {};
	struct dummy_keys {};

	using namespace window::event;
	using namespace mouse;

	luabind::scope _all_components() {
		return
			bind_map_wrapper<int, animation*>("animate_info"),

			luabind::class_<animate>("animate_component")
			.def(luabind::constructor<>())
			.def_readwrite("available_animations", &animate::available_animations)
			.def("set_current_animation_set", &animate::set_current_animation_set),

			luabind::class_<behaviour_tree::decorator>("behaviour_decorator")
			.def(luabind::constructor<>())
			.def_readwrite("next_decorator", &behaviour_tree::decorator::next_decorator)
			,

			luabind::class_<behaviour_tree::timer_decorator, behaviour_tree::decorator>("behaviour_timer_decorator")
			.def(luabind::constructor<>())
			.def_readwrite("maximum_running_time_ms", &behaviour_tree::timer_decorator::maximum_running_time_ms)
			,

			luabind::class_<behaviour_tree::task>("behaviour_tree_task")
			.def(luabind::constructor<>())
			.def("interrupt_runner", &behaviour_tree::task::interrupt_runner)
			.def("interrupt_other_runner", &behaviour_tree::task::interrupt_other_runner)
			,

			luabind::class_<behaviour_tree::composite>("behaviour_node")
			.def(luabind::constructor<>())
			.def_readwrite("default_return", &behaviour_tree::composite::default_return)
			.def_readwrite("node_type", &behaviour_tree::composite::node_type)
			.def_readwrite("on_enter", &behaviour_tree::composite::enter_callback)
			.def_readwrite("on_exit", &behaviour_tree::composite::exit_callback)
			.def_readwrite("on_update", &behaviour_tree::composite::update_callback)
			.def_readwrite("name", &behaviour_tree::composite::name)
			.def_readwrite("decorator_chain", &behaviour_tree::composite::decorator_chain)
			.def_readwrite("concurrent_return", &behaviour_tree::composite::concurrent_return)
			.def_readwrite("skip_to_running_child", &behaviour_tree::composite::skip_to_running_child)
			.def("add_child", &behaviour_tree::composite::add_child)
			.enum_("constants")[
				luabind::value("SUCCESS", behaviour_tree::composite::SUCCESS),
					luabind::value("RUNNING", behaviour_tree::composite::RUNNING),
					luabind::value("FAILURE", behaviour_tree::composite::FAILURE),
					luabind::value("INTERRUPTED", behaviour_tree::composite::INTERRUPTED),
					luabind::value("SEQUENCER", behaviour_tree::composite::SEQUENCER),
					luabind::value("SELECTOR", behaviour_tree::composite::SELECTOR),
					luabind::value("CONCURRENT", behaviour_tree::composite::CONCURRENT)
			],

			luabind::class_<behaviour_tree>("behaviour_tree_component")
			.def(luabind::constructor<>())
			.def("add_tree", &behaviour_tree::add_tree),

			luabind::class_<camera>("camera_component")
			.def(luabind::constructor<>())
			.def_readwrite("screen_rect", &camera::screen_rect)
			.def_readwrite("size", &camera::size)
			.def_readwrite("layer", &camera::layer)
			.def_readwrite("mask", &camera::mask)
			.def_readwrite("enabled", &camera::enabled)
			.def_readwrite("orbit_mode", &camera::orbit_mode)
			.def_readwrite("angled_look_length", &camera::angled_look_length)
			.def_readwrite("enable_smoothing", &camera::enable_smoothing)
			.def_readwrite("dont_smooth_once", &camera::dont_smooth_once)
			.def_readwrite("smoothing_average_factor", &camera::smoothing_average_factor)
			.def_readwrite("averages_per_sec", &camera::averages_per_sec)
			.def_readwrite("last_interpolant", &camera::last_interpolant)
			.def_readwrite("max_look_expand", &camera::max_look_expand)
			.def_readwrite("player", &camera::player)
			.def_readwrite("crosshair", &camera::crosshair)
			.property("drawing_callback", bind_callback(&camera::drawing_callback))
			.def_readwrite("crosshair_follows_interpolant", &camera::crosshair_follows_interpolant)
			.enum_("orbit_type")[
				luabind::value("NONE", camera::orbit_type::NONE),
					luabind::value("ANGLED", camera::orbit_type::ANGLED),
					luabind::value("LOOK", camera::orbit_type::LOOK)
			],

			luabind::class_<position_copying>("position_copying_component")
					.def(luabind::constructor<>())
					.def("set_target", &position_copying::set_target)
					.def_readwrite("target", &position_copying::target)
					.def_readwrite("position_copying_type", &position_copying::position_copying_type)
					.def_readwrite("reference_position", &position_copying::reference_position)
					.def_readwrite("target_reference_position", &position_copying::target_reference_position)
					.def_readwrite("scrolling_speed", &position_copying::scrolling_speed)
					.def_readwrite("offset", &position_copying::offset)
					.def_readwrite("rotation_orbit_offset", &position_copying::rotation_orbit_offset)
					.def_readwrite("rotation_offset", &position_copying::rotation_offset)
					.def_readwrite("relative", &position_copying::relative)
					.def_readwrite("position_copying_rotation", &position_copying::position_copying_rotation)
					.def_readwrite("track_origin", &position_copying::track_origin)
					.def_readwrite("rotation_multiplier", &position_copying::rotation_multiplier)
					.def_readwrite("subscribe_to_previous", &position_copying::subscribe_to_previous)
					.enum_("position_copying_type")[
						luabind::value("OFFSET", position_copying::position_copying_type::OFFSET),
							luabind::value("PARALLAX", position_copying::position_copying_type::PARALLAX),
							luabind::value("ORBIT", position_copying::position_copying_type::ORBIT)
					],

					luabind::class_<children>("children_component")
							.def(luabind::constructor<>())
							.def("add", &children::add),

							luabind::class_<crosshair>("crosshair_component")
							.def(luabind::constructor<>())
							//.def_readwrite("bounds", &components::crosshair::bounds)
							//.def_readwrite("blink", &components::crosshair::blink)
							.def_readwrite("should_blink", &crosshair::should_blink)
							.def_readwrite("size_multiplier", &crosshair::size_multiplier)
							.def_readwrite("rotation_offset", &crosshair::rotation_offset)
							.def_readwrite("sensitivity", &crosshair::sensitivity),


							luabind::class_<damage>("damage_component")
							.def(luabind::constructor<>())
							.def_readwrite("amount", &damage::amount)
							.def_readwrite("sender", &damage::sender)
							.def_readwrite("starting_point", &damage::starting_point)
							.def_readwrite("lifetime", &damage::lifetime)
							.def_readwrite("max_lifetime_ms", &damage::max_lifetime_ms)
							.def_readwrite("destroy_upon_hit", &damage::destroy_upon_hit)
							.def_readwrite("max_distance", &damage::max_distance),


							luabind::class_<gun>("gun_component")
							.def(luabind::constructor<>())
							.def(luabind::constructor<const gun&>())
							.def("transfer_barrel_smoke", &gun::transfer_barrel_smoke)
							.def("get_barrel_smoke", &gun::get_barrel_smoke)
							.def("set_bullet_filter", &gun::set_bullet_filter)
							.def("drop_logic", &gun::drop_logic)
							.def_readwrite("current_rounds", &gun::current_rounds)
							.def_readwrite("trigger_mode", &gun::trigger_mode)
							.def_readwrite("current_state", &gun::current_state)
							.def_readwrite("target_camera_to_shake", &gun::target_camera_to_shake)
							.def_readwrite("bullets_once", &gun::bullets_once)
							.def_readwrite("max_rounds", &gun::max_rounds)
							.def_readwrite("spread_degrees", &gun::spread_degrees)
							.def_readwrite("bullet_damage", &gun::bullet_damage)
							.def_readwrite("bullet_speed", &gun::bullet_speed)
							.def_readwrite("shooting_interval_ms", &gun::shooting_interval_ms)
							.def_readwrite("max_bullet_distance", &gun::max_bullet_distance)
							.def_readwrite("melee_filter", &gun::melee_filter)
							.def_readwrite("melee_obstruction_filter", &gun::melee_obstruction_filter)
							.def_readwrite("bullet_distance_offset", &gun::bullet_distance_offset)
							.def_readwrite("shake_radius", &gun::shake_radius)
							.def_readwrite("shake_spread_degrees", &gun::shake_spread_degrees)
							.def_readwrite("is_automatic", &gun::is_automatic)
							.def_readwrite("bullet_render", &gun::bullet_render)
							.def_readwrite("swing_duration", &gun::swing_duration)
							.def_readwrite("swing_radius", &gun::swing_radius)
							.def_readwrite("swing_angle", &gun::swing_angle)
							.def_readwrite("swing_angular_offset", &gun::swing_angular_offset)
							.def_readwrite("swing_interval_ms", &gun::swing_interval_ms)
							//.def_readwrite("query_vertices", &gun::query_vertices)
							.def_readwrite("bullet_body", &gun::bullet_body)
							.enum_("constants")[
								luabind::value("NONE", gun::NONE),
									luabind::value("MELEE", gun::MELEE),
									luabind::value("SHOOT", gun::SHOOT),
									luabind::value("READY", gun::READY),
									luabind::value("SWINGING", gun::SWINGING),
									luabind::value("SWINGING_INTERVAL", gun::SWINGING_INTERVAL),
									luabind::value("SHOOTING_INTERVAL", gun::SHOOTING_INTERVAL)
							],

							luabind::class_<input_system::context>("input_context")
									.def(luabind::constructor<>())
									.def("set_intent", &input_system::context::set_intent)
									.def_readwrite("enabled", &input_system::context::enabled),



									luabind::class_<input>("input_component")
									.def(luabind::constructor<>())
									.def("add", &input::add),

									luabind::class_<dummy_mouse>("mouse")
									.enum_("constants")
									[
										luabind::value("ltripleclick", ltripleclick),
										luabind::value("raw_motion", raw_motion),
									luabind::value("motion", motion),
									luabind::value("wheel", wheel),
									luabind::value("ldoubleclick", ldoubleclick),
									luabind::value("mdoubleclick", mdoubleclick),
									luabind::value("rdoubleclick", rdoubleclick),
									luabind::value("ldown", ldown),
									luabind::value("lup", lup),
									luabind::value("mdown", mdown),
									luabind::value("mup", mup),
									luabind::value("rdown", rdown),
									luabind::value("rup", rup)
									],

									luabind::class_<dummy_keys>("keys")
									.enum_("constants")
									[
										luabind::value("LMOUSE", keys::LMOUSE),
										luabind::value("RMOUSE", keys::RMOUSE),
									luabind::value("MMOUSE", keys::MMOUSE),
									luabind::value("CANCEL", keys::CANCEL),
									luabind::value("BACKSPACE", keys::BACKSPACE),
									luabind::value("TAB", keys::TAB),
									luabind::value("CLEAR", keys::CLEAR),
									luabind::value("ENTER", keys::ENTER),
									luabind::value("SHIFT", keys::SHIFT),
									luabind::value("CTRL", keys::CTRL),
									luabind::value("ALT", keys::ALT),
									luabind::value("PAUSE", keys::PAUSE),
									luabind::value("CAPSLOCK", keys::CAPSLOCK),
									luabind::value("ESC", keys::ESC),
									luabind::value("SPACE", keys::SPACE),
									luabind::value("PAGEUP", keys::PAGEUP),
									luabind::value("PAGEDOWN", keys::PAGEDOWN),
									luabind::value("END", keys::END),
									luabind::value("HOME", keys::HOME),
									luabind::value("LEFT", keys::LEFT),
									luabind::value("UP", keys::UP),
									luabind::value("RIGHT", keys::RIGHT),
									luabind::value("DOWN", keys::DOWN),
									luabind::value("SELECT", keys::SELECT),
									luabind::value("PRINT", keys::PRINT),
									luabind::value("EXECUTE", keys::EXECUTE),
									luabind::value("PRINTSCREEN", keys::PRINTSCREEN),
									luabind::value("INSERT", keys::INSERT),
									luabind::value("DEL", keys::DEL),
									luabind::value("HELP", keys::HELP),
									luabind::value("LWIN", keys::LWIN),
									luabind::value("RWIN", keys::RWIN),
									luabind::value("APPS", keys::APPS),
									luabind::value("SLEEP", keys::SLEEP),
									luabind::value("NUMPAD0", keys::NUMPAD0),
									luabind::value("NUMPAD1", keys::NUMPAD1),
									luabind::value("NUMPAD2", keys::NUMPAD2),
									luabind::value("NUMPAD3", keys::NUMPAD3),
									luabind::value("NUMPAD4", keys::NUMPAD4),
									luabind::value("NUMPAD5", keys::NUMPAD5),
									luabind::value("NUMPAD6", keys::NUMPAD6),
									luabind::value("NUMPAD7", keys::NUMPAD7),
									luabind::value("NUMPAD8", keys::NUMPAD8),
									luabind::value("NUMPAD9", keys::NUMPAD9),
									luabind::value("MULTIPLY", keys::MULTIPLY),
									luabind::value("ADD", keys::ADD),
									luabind::value("SEPARATOR", keys::SEPARATOR),
									luabind::value("SUBTRACT", keys::SUBTRACT),
									luabind::value("DECIMAL", keys::DECIMAL),
									luabind::value("DIVIDE", keys::DIVIDE),
									luabind::value("F1", keys::F1),
									luabind::value("F2", keys::F2),
									luabind::value("F3", keys::F3),
									luabind::value("F4", keys::F4),
									luabind::value("F5", keys::F5),
									luabind::value("F6", keys::F6),
									luabind::value("F7", keys::F7),
									luabind::value("F8", keys::F8),
									luabind::value("F9", keys::F9),
									luabind::value("F10", keys::F10),
									luabind::value("F11", keys::F11),
									luabind::value("F12", keys::F12),
									luabind::value("F13", keys::F13),
									luabind::value("F14", keys::F14),
									luabind::value("F15", keys::F15),
									luabind::value("F16", keys::F16),
									luabind::value("F17", keys::F17),
									luabind::value("F18", keys::F18),
									luabind::value("F19", keys::F19),
									luabind::value("F20", keys::F20),
									luabind::value("F21", keys::F21),
									luabind::value("F22", keys::F22),
									luabind::value("F23", keys::F23),
									luabind::value("F24", keys::F24),
									luabind::value("A", keys::A),
									luabind::value("B", keys::B),
									luabind::value("C", keys::C),
									luabind::value("D", keys::D),
									luabind::value("E", keys::E),
									luabind::value("F", keys::F),
									luabind::value("G", keys::G),
									luabind::value("H", keys::H),
									luabind::value("I", keys::I),
									luabind::value("J", keys::J),
									luabind::value("K", keys::K),
									luabind::value("L", keys::L),
									luabind::value("M", keys::M),
									luabind::value("N", keys::N),
									luabind::value("O", keys::O),
									luabind::value("P", keys::P),
									luabind::value("Q", keys::Q),
									luabind::value("R", keys::R),
									luabind::value("S", keys::S),
									luabind::value("T", keys::T),
									luabind::value("U", keys::U),
									luabind::value("V", keys::V),
									luabind::value("W", keys::W),
									luabind::value("X", keys::X),
									luabind::value("Y", keys::Y),
									luabind::value("Z", keys::Z),
									luabind::value("_0", keys::_0),
									luabind::value("_1", keys::_1),
									luabind::value("_2", keys::_2),
									luabind::value("_3", keys::_3),
									luabind::value("_4", keys::_4),
									luabind::value("_5", keys::_5),
									luabind::value("_6", keys::_6),
									luabind::value("_7", keys::_7),
									luabind::value("_8", keys::_8),
									luabind::value("_9", keys::_9),
									luabind::value("NUMLOCK", keys::NUMLOCK),
									luabind::value("SCROLL", keys::SCROLL),
									luabind::value("LSHIFT", keys::LSHIFT),
									luabind::value("RSHIFT", keys::RSHIFT),
									luabind::value("LCTRL", keys::LCTRL),
									luabind::value("RCTRL", keys::RCTRL),
									luabind::value("LALT", keys::LALT),
									luabind::value("RALT", keys::RALT),
									luabind::value("DASH", keys::DASH)
									],


									luabind::class_<rotation_copying>("rotation_copying_component")
									.def(luabind::constructor<>())
									.def_readwrite("look_mode", &rotation_copying::look_mode)
									.def_readwrite("target", &rotation_copying::target)
									.def_readwrite("easing_mode", &rotation_copying::easing_mode)
									.def_readwrite("smoothing_average_factor", &rotation_copying::smoothing_average_factor)
									.def_readwrite("averages_per_sec", &rotation_copying::averages_per_sec)
									.def_readwrite("update_value", &rotation_copying::update_value)
									.def_readwrite("last_value", &rotation_copying::last_value)
									.enum_("position_copying_type")[
										luabind::value("POSITION", rotation_copying::look_type::POSITION),
											luabind::value("VELOCITY", rotation_copying::look_type::VELOCITY),
											luabind::value("ACCELEARATION", rotation_copying::look_type::ACCELEARATION),
											luabind::value("NONE", rotation_copying::rotation_copying_easing::NONE),
											luabind::value("LINEAR", rotation_copying::rotation_copying_easing::LINEAR),
											luabind::value("EXPONENTIAL", rotation_copying::rotation_copying_easing::EXPONENTIAL)
									],

										luabind::class_<movement>("movement_component")
											.def(luabind::constructor<>())
											.def("add_animation_receiver", &movement::add_animation_receiver)
											.def_readwrite("force_offset", &movement::force_offset)
											.def_readwrite("animation_message", &movement::animation_message)
											.def_readwrite("moving_left", &movement::moving_left)
											.def_readwrite("moving_right", &movement::moving_right)
											.def_readwrite("moving_forward", &movement::moving_forward)
											.def_readwrite("moving_backward", &movement::moving_backward)
											.def_readwrite("axis_rotation_degrees", &movement::axis_rotation_degrees)
											.def_readwrite("input_acceleration", &movement::input_acceleration)
											.def_readwrite("braking_damping", &movement::braking_damping)
											.def_readwrite("max_speed_animation", &movement::max_speed_animation)
											.def_readwrite("inverse_thrust_brake", &movement::inverse_thrust_brake)
											.def_readwrite("max_speed", &movement::max_speed)
											.def_readwrite("thrust_parallel_to_ground_length", &movement::thrust_parallel_to_ground_length)
											.def_readwrite("ground_filter", &movement::ground_filter)
											.def_readwrite("requested_movement", &movement::requested_movement)
											.def_readwrite("max_accel_len", &movement::max_accel_len)
											.def_readwrite("sidescroller_setup", &movement::sidescroller_setup)
											.def_readwrite("air_resistance", &movement::air_resistance),


											bind_map_wrapper<int, particle_effect>("particle_emitter_info"),

											luabind::class_<components::particle_group>("particle_group_component")
											.def(luabind::constructor<>())
											.def_readwrite("pause_emission", &components::particle_group::pause_emission)
											,

											luabind::class_<components::particle_emitter>("particle_emitter_component")
											.def(luabind::constructor<>())
											.def(luabind::constructor<particle_emitter_info*>())
											.def_readwrite("available_particle_effects", &components::particle_emitter::available_particle_effects)
											.def("get_effect", &components::particle_emitter::get_effect),


											luabind::class_<pathfinding::navigation_hint>("navigation_hint")
											.def(luabind::constructor<>())
											.def_readwrite("enabled", &pathfinding::navigation_hint::enabled)
											.def_readwrite("origin", &pathfinding::navigation_hint::origin)
											.def_readwrite("target", &pathfinding::navigation_hint::target)
											,

											luabind::class_<pathfinding>("pathfinding_component")
											.def(luabind::constructor<>())
											.def("start_pathfinding", &pathfinding::start_pathfinding)
											.def("start_exploring", &pathfinding::start_exploring)
											.def("is_still_exploring", &pathfinding::is_still_exploring)
											.def("get_current_navigation_target", &pathfinding::get_current_navigation_target)
											.def("get_current_target", &pathfinding::get_current_target)
											.def("clear_pathfinding_info", &pathfinding::clear_pathfinding_info)
											.def("is_still_pathfinding", &pathfinding::is_still_pathfinding)
											.def("exists_through_undiscovered_visible", &pathfinding::exists_through_undiscovered_visible)
											.def("reset_persistent_navpoint", &pathfinding::reset_persistent_navpoint)
											.def("clear_internal_data", &pathfinding::clear_internal_data)
											.def_readwrite("custom_exploration_hint", &pathfinding::custom_exploration_hint)
											.def_readwrite("favor_velocity_parallellness", &pathfinding::favor_velocity_parallellness)
											.def_readwrite("enable_backtracking", &pathfinding::enable_backtracking)
											.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
											.def_readwrite("rotate_navpoints", &pathfinding::rotate_navpoints)
											.def_readwrite("target_offset", &pathfinding::target_offset)
											.def_readwrite("eye_offset", &pathfinding::eye_offset)
											.def_readwrite("force_touch_sensors", &pathfinding::force_touch_sensors)
											.def_readwrite("force_persistent_navpoints", &pathfinding::force_persistent_navpoints)
											.def_readwrite("distance_navpoint_hit", &pathfinding::distance_navpoint_hit)
											.def_readwrite("starting_ignore_discontinuities_shorter_than", &pathfinding::starting_ignore_discontinuities_shorter_than)
											.def_readwrite("enable_session_rollbacks", &pathfinding::enable_session_rollbacks)
											.def_readwrite("mark_touched_as_discovered", &pathfinding::mark_touched_as_discovered),

											luabind::def("SetDensity", set_density),
											luabind::def("SetFilter", set_filter),
											luabind::def("SetFriction", set_friction),
											luabind::def("SetRestitution", set_restitution),
											luabind::class_<dummy_Box2D>("Box2D")
											.enum_("constants")[
												luabind::value("b2_dynamicBody", b2_dynamicBody),
													luabind::value("b2_staticBody", b2_staticBody),
													luabind::value("b2_kinematicBody", b2_kinematicBody)
											],

											luabind::class_<physics_system::raycast_output>("raycast_output")
													.def(luabind::constructor<>())
													.def_readwrite("intersection", &physics_system::raycast_output::intersection)
													.def_readwrite("hit", &physics_system::raycast_output::hit)
													.def_readwrite("normal", &physics_system::raycast_output::normal)
													.def_readwrite("what_entity", &physics_system::raycast_output::what_entity)
													,

													luabind::class_<physics_system::query_output::queried_result>("queried_result")
													.def(luabind::constructor<>())
													.def_readwrite("location", &physics_system::query_output::queried_result::location)
													.def_readwrite("body", &physics_system::query_output::queried_result::body)
													,

													luabind::class_<physics_system::query_output>("query_output")
													.def(luabind::constructor<>())
													.def_readwrite("bodies", &physics_system::query_output::bodies, luabind::return_stl_iterator())
													.def_readwrite("details", &physics_system::query_output::details, luabind::return_stl_iterator())
													,

													luabind::class_<b2World>("b2World")
													.def("SetGravity", &b2World::SetGravity)
													,

													luabind::class_<b2Shape>("b2Shape"),
													luabind::class_<b2PolygonShape, b2Shape>("b2PolygonShape")
													.def(luabind::constructor<>())
													.def("SetAsBox", (void(__thiscall b2PolygonShape::*)(float32, float32))(&b2PolygonShape::SetAsBox))
													,

													luabind::class_<physics>("physics_component")
													.def(luabind::constructor<>())
													.def_readwrite("body", &physics::body)
													.def_readwrite("target_angle", &physics::target_angle)
													.def_readwrite("enable_angle_motor", &physics::enable_angle_motor)
													.def_readwrite("angle_motor_force_multiplier", &physics::angle_motor_force_multiplier)

													,

													luabind::class_<b2Vec2>("b2Vec2")
													.def(luabind::constructor<float, float>())
													.def(luabind::constructor<const b2Vec2&>())
													.def(luabind::constructor<>())
													.def("LengthSquared", &b2Vec2::LengthSquared)
													.def("Length", &b2Vec2::Length)
													.def_readwrite("x", &b2Vec2::x)
													.def_readwrite("y", &b2Vec2::y),

													luabind::class_<b2Body>("b2Body")
													.def("ApplyForce", &b2Body::ApplyForce)
													.def("ApplyLinearImpulse", &b2Body::ApplyLinearImpulse)
													.def("GetWorldCenter", &b2Body::GetWorldCenter)
													.def("SetFixedRotation", &b2Body::SetFixedRotation)
													.def("SetGravityScale", &b2Body::SetGravityScale)
													.def("SetLinearDamping", &b2Body::SetLinearDamping)
													.def("SetLinearVelocity", &b2Body::SetLinearVelocity)
													.def("SetAngularDamping", &b2Body::GetAngularDamping)
													.def("ApplyAngularImpulse", &b2Body::ApplyAngularImpulse)
													.def("ApplyTorque", &b2Body::ApplyTorque)
													.def("GetLinearVelocity", &b2Body::GetLinearVelocity)
													.def("GetAngularVelocity", &b2Body::GetAngularVelocity)
													.def("SetAngularVelocity", &b2Body::SetAngularVelocity)
													.def("SetMaximumLinearVelocity", &b2Body::SetMaximumLinearVelocity)
													.def("SetTransform", &b2Body::SetTransform)
													.def("GetPosition", &b2Body::GetPosition)
													.def("GetAngle", &b2Body::GetAngle)
													.def("GetMass", &b2Body::GetMass)
													.def("SetBullet", &b2Body::SetBullet),

													luabind::class_<renderer::debug_line>("debug_line")
													.def(luabind::constructor<vec2, vec2, pixel_32>())
													,
													luabind::class_<augs::vertex_triangle_buffer>("triangle_buffer"),

													luabind::class_<render>("render_component")
													.def(luabind::constructor<>())
													.def_readwrite("mask", &render::mask)
													.def_readwrite("layer", &render::layer)
													.def_readwrite("last_screen_pos", &render::last_screen_pos)
													.def_readwrite("was_drawn", &render::was_drawn)
													.def_readwrite("flip_horizontally", &render::flip_horizontally)
													.def_readwrite("flip_vertically", &render::flip_vertically)
													.def_readwrite("absolute_transform", &render::absolute_transform)
													.enum_("mask_type")[
														luabind::value("WORLD", render::mask_type::WORLD),
															luabind::value("GUI", render::mask_type::GUI)
													],

														luabind::class_<steering::target_info>("target_info")
															.def(luabind::constructor<>())
															.def("set", (void (steering::target_info::*)(entity_id))&steering::target_info::set)
															.def("set", (void (steering::target_info::*)(vec2, vec2))&steering::target_info::set)
															.def_readwrite("is_set", &steering::target_info::is_set)
															,

															luabind::class_<steering::behaviour_state>("behaviour_state")
															.def(luabind::constructor<steering::behaviour*>())
															.def_readwrite("subject_behaviour", &steering::behaviour_state::subject_behaviour)
															.def_readwrite("target", &steering::behaviour_state::target)
															.def_readwrite("target_from", &steering::behaviour_state::target_from)
															.def_readwrite("last_output_force", &steering::behaviour_state::last_output_force)
															.def_readwrite("last_estimated_target_position", &steering::behaviour_state::last_estimated_target_position)
															.def_readwrite("enabled", &steering::behaviour_state::enabled)
															.def_readwrite("weight_multiplier", &steering::behaviour_state::weight_multiplier)
															.def_readwrite("current_wander_angle", &steering::behaviour_state::current_wander_angle),

															luabind::class_<steering::behaviour>("steering_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("force_color", &steering::behaviour::force_color)
															.def_readwrite("max_force_applied", &steering::behaviour::max_force_applied)
															.def_readwrite("weight", &steering::behaviour::weight),


															luabind::class_<steering::directed, steering::behaviour>("directed_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("radius_of_effect", &steering::directed::radius_of_effect)
															.def_readwrite("max_target_future_prediction_ms", &steering::directed::max_target_future_prediction_ms),

															luabind::class_<steering::avoidance, steering::behaviour>("avoidance_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("intervention_time_ms", &steering::avoidance::intervention_time_ms)
															.def_readwrite("max_intervention_length", &steering::avoidance::max_intervention_length)
															.def_readwrite("avoidance_rectangle_width", &steering::avoidance::avoidance_rectangle_width)
															,

															luabind::class_<steering::flocking, steering::behaviour>("flocking_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("group", &steering::flocking::group)
															.def_readwrite("square_side", &steering::flocking::square_side)
															.def_readwrite("field_of_vision_degrees", &steering::flocking::field_of_vision_degrees)
															,

															luabind::class_<steering::separation, steering::flocking>("separation_behaviour")
															.def(luabind::constructor<>())
															,

															luabind::class_<steering::seek, steering::directed>("seek_behaviour")
															.def(luabind::constructor<>()),

															luabind::class_<steering::flee, steering::directed>("flee_behaviour")
															.def(luabind::constructor<>()),

															luabind::class_<steering::wander, steering::behaviour>("wander_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("circle_radius", &steering::wander::circle_radius)
															.def_readwrite("circle_distance", &steering::wander::circle_distance)
															.def_readwrite("displacement_degrees", &steering::wander::displacement_degrees)
															,

															luabind::class_<steering::containment, steering::avoidance>("containment_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("randomize_rays", &steering::containment::randomize_rays)
															.def_readwrite("only_threats_in_OBB", &steering::containment::only_threats_in_OBB)
															.def_readwrite("ray_count", &steering::containment::ray_count)
															.def_readwrite("ray_filter", &steering::containment::ray_filter)
															,

															luabind::class_<steering::obstacle_avoidance, steering::avoidance>("obstacle_avoidance_behaviour")
															.def(luabind::constructor<>())
															.def_readwrite("navigation_correction", &steering::obstacle_avoidance::navigation_correction)
															.def_readwrite("navigation_seek", &steering::obstacle_avoidance::navigation_seek)
															.def_readwrite("ignore_discontinuities_narrower_than", &steering::obstacle_avoidance::ignore_discontinuities_narrower_than)
															.def_readwrite("visibility_type", &steering::obstacle_avoidance::visibility_type)
															,

															luabind::class_<steering>("steering_component")
															.def(luabind::constructor<>())
															.def_readwrite("last_resultant_force", &steering::last_resultant_force)
															.def_readwrite("apply_force", &steering::apply_force)
															.def("add_behaviour", &steering::add_behaviour)
															.def("clear_behaviours", &steering::clear_behaviours)
															.def_readwrite("max_speed", &steering::max_speed)
															.def_readwrite("max_resultant_force", &steering::max_resultant_force),

															luabind::class_<transform>("transform_component")
															.def(luabind::constructor<>())
															.def(luabind::constructor<const transform&>())
															.def_readwrite("pos", &transform::pos)
															.def_readwrite("rotation", &transform::rotation),

															luabind::class_<visibility::edge>("visibility_edge")
															.def_readwrite("first", &visibility::edge::first)
															.def_readwrite("second", &visibility::edge::second),

															luabind::class_<visibility::discontinuity>("visibility_discontinuity")
															.def(luabind::constructor<>())
															.def_readwrite("points", &visibility::discontinuity::points)
															.def_readwrite("winding", &visibility::discontinuity::winding),


															luabind::class_<visibility::layer>("visibility_layer")
															.def(luabind::constructor<>())
															.def_readwrite("filter", &visibility::layer::filter)
															.def_readwrite("square_side", &visibility::layer::square_side)
															.def_readwrite("color", &visibility::layer::color)
															.def_readwrite("ignore_discontinuities_shorter_than", &visibility::layer::ignore_discontinuities_shorter_than)
															.def_readwrite("offset", &visibility::layer::offset)
															.def_readwrite("postprocessing_subject", &visibility::layer::postprocessing_subject)
															.def("get_polygon", &visibility::layer::get_polygon)
															.def("get_discontinuity", &visibility::layer::get_discontinuity)
															.def("get_num_discontinuities", &visibility::layer::get_num_discontinuities)
															,

															luabind::class_<visibility>("visibility_component")
															.def(luabind::constructor<>())
															.def("add_layer", &visibility::add_layer)
															.def("get_layer", &visibility::get_layer)
															.def_readwrite("interval_ms", &visibility::interval_ms)
															.enum_("constants")
															[
																luabind::value("OBSTACLE_AVOIDANCE", visibility::OBSTACLE_AVOIDANCE),
																luabind::value("CONTAINMENT", visibility::CONTAINMENT),
															luabind::value("DYNAMIC_PATHFINDING", visibility::DYNAMIC_PATHFINDING)
															]





									;
	}
}