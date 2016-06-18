#include "cosmos.h"

#include "systems/physics_system.h"
#include "systems/steering_system.h"
#include "systems/movement_system.h"
#include "systems/visibility_system.h"
#include "systems/pathfinding_system.h"
#include "systems/animation_system.h"
#include "systems/camera_system.h"
#include "systems/render_system.h"
#include "systems/input_system.h"
#include "systems/gun_system.h"
#include "systems/crosshair_system.h"
#include "systems/rotation_copying_system.h"
#include "systems/position_copying_system.h"
#include "systems/damage_system.h"
#include "systems/destroy_system.h"
#include "systems/particles_system.h"
#include "systems/behaviour_tree_system.h"
#include "systems/car_system.h"
#include "systems/driver_system.h"
#include "systems/trigger_detector_system.h"
#include "systems/item_system.h"
#include "systems/force_joint_system.h"
#include "systems/intent_contextualization_system.h"
#include "systems/gui_system.h"
#include "systems/trace_system.h"
#include "systems/melee_system.h"
#include "systems/sentience_system.h"
#include "systems/dynamic_tree_system.h"

#include "game/globals/render_layer.h"

#include "cosmos.h"

#include "game/step_state.h"
#include "game/cosmic_profiler.h"

using namespace components;
using namespace messages;

void cosmos::call_rendering_schemata(augs::variable_delta delta, cosmic_profiler& profiler) const {
	step_state step;
	const auto& cosm = *this;

	auto& performance = profiler.performance;

	performance.start(meter_type::RENDERING);

	performance.start(meter_type::CAMERA_QUERY);
	render_system().determine_visible_entities_from_every_camera();
	performance.stop(meter_type::CAMERA_QUERY);

	performance.start(meter_type::INTERPOLATION);
	render_system().calculate_and_set_interpolated_transforms();
	performance.stop(meter_type::INTERPOLATION);
	
	input_system().post_unmapped_intents_from_raw_window_inputs();
	input_system().map_unmapped_intents_to_entities();

	movement_system().generate_movement_responses();

	animation_system().game_responses_to_animation_messages();

	animation_system().handle_animation_messages();
	animation_system().progress_animation_states();


	position_copying_system().update_transforms();
	camera_system().resolve_cameras_transforms_and_smoothing();
	rotation_copying_system().update_rotations();

	camera_system().post_render_requests_for_all_cameras();

	input_system().acquire_new_events_posted_by_drawing_time_systems();
	
	performance.stop(meter_type::RENDERING);
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", entities_count());
}

std::vector<entity_id> cosmos::get_list(processing_subjects list) const {
	return lists_of_processing_subjects.get(list);
}

void cosmos::advance_deterministic_schemata(augs::machine_entropy input, cosmic_profiler& profiler) {
	step_state step;
	auto& cosm = *this;
	auto& performance = profiler.performance;

	stateful_systems.get<gui_system>().rebuild_gui_tree_based_on_game_state();
	stateful_systems.get<gui_system>().translate_raw_window_inputs_to_gui_events();

	stateful_systems.get<gui_system>().suppress_inputs_meant_for_gui();

	performance.start(meter_type::LOGIC);
	render_system().set_current_transforms_as_previous_for_interpolation();

	input_system().post_all_events_posted_by_drawing_time_systems_since_last_step();
	
	stateful_systems.get<gui_system>().switch_to_gui_mode_and_back();

	input_system().map_unmapped_intents_to_entities();

	intent_contextualization_system().contextualize_crosshair_action_intents();

	gun_system().consume_gun_intents();
	gun_system().launch_shots_due_to_pressed_triggers();

	particles_system().create_particle_effects();

	render_system().add_entities_to_rendering_tree();
	stateful_systems.get<physics_system>().react_to_new_entities();
	step.messages.get_queue<new_entity_message>().clear();

	trace_system().lengthen_sprites_of_traces();

	crosshair_system().generate_crosshair_intents(step);
	crosshair_system().apply_crosshair_intents_to_base_offsets(cosm, step);
	crosshair_system().apply_base_offsets_to_crosshair_transforms(cosm, step);
	crosshair_system().animate_crosshair_sizes(cosm);

	camera_system().react_to_input_intents();

	/* intent delegation stage (various ownership relations) */
	intent_contextualization_system().contextualize_use_button_intents();
	/* end of intent delegation stage */

	driver_system().release_drivers_due_to_requests();

	trigger_detector_system().consume_trigger_detector_presses();
	trigger_detector_system().post_trigger_requests_from_continuous_detectors();
	trigger_detector_system().send_trigger_confirmations();

	item_system().translate_gui_intents_to_transfer_requests();
	item_system().handle_trigger_confirmations_as_pick_requests();
	item_system().handle_holster_item_intents();
	item_system().handle_throw_item_intents();

	driver_system().assign_drivers_from_successful_trigger_hits();
	driver_system().release_drivers_due_to_ending_contact_with_wheel();

	intent_contextualization_system().contextualize_movement_intents();

	melee_system().consume_melee_intents();
	force_joint_system().apply_forces_towards_target_entities();

	car_system().set_steering_flags_from_intents();
	car_system().apply_movement_forces();

	movement_system().set_movement_flags_from_input();
	movement_system().apply_movement_forces();

	rotation_copying_system().update_physical_motors();
	performance.start(meter_type::PHYSICS);
	stateful_systems.get<physics_system>().execute_delayed_physics_ops();

	raycasts.measure(stateful_systems.get<physics_system>().ray_casts_since_last_step);

	stateful_systems.get<physics_system>().step_and_set_new_transforms();
	performance.stop(meter_type::PHYSICS);
	position_copying_system().update_transforms();
	
	melee_system().initiate_and_update_moves();

	damage_system().destroy_outdated_bullets();
	damage_system().destroy_colliding_bullets_and_send_damage();

	sentience_system().apply_damage_and_generate_health_events();
	sentience_system().cooldown_aimpunches();
	
	particles_system().game_responses_to_particle_effects();
	particles_system().create_particle_effects();

	stateful_systems.get<gui_system>().translate_game_events_for_hud();

	performance.start(meter_type::VISIBILITY);
	visibility_system().generate_visibility_and_sight_information();
	performance.stop(meter_type::VISIBILITY);

	performance.start(meter_type::AI);
	behaviour_tree_system().evaluate_trees();
	performance.stop(meter_type::AI);

	performance.start(meter_type::PATHFINDING);
	pathfinding_system().advance_pathfinding_sessions(cosm);
	performance.stop(meter_type::PATHFINDING);

	particles_system().step_streams_and_particles();
	particles_system().destroy_dead_streams();
	trace_system().destroy_outdated_traces();

	destroy_system().queue_children_of_queued_entities();

	trace_system().spawn_finishing_traces_for_destroyed_objects();
	render_system().remove_entities_from_rendering_tree();
	stateful_systems.get<physics_system>().react_to_destroyed_entities();

	bool has_no_destruction_callback_queued_any_additional_destruction = step.messages.get_queue<messages::queue_destruction>().empty();
	ensure(has_no_destruction_callback_queued_any_additional_destruction);

	destroy_system().perform_deletions();

	step.messages.clear_queue<messages::collision_message>(); 
	
	step.messages.ensure_all_empty();

	++current_step_number;
	seconds_passed += delta.in_seconds();
	performance.stop(meter_type::LOGIC);
}

cosmos::cosmos() {
	stateful_systems.create<gui_system>(std::ref(*this));
	stateful_systems.create<dynamic_tree_system>(std::ref(*this));
	stateful_systems.create<physics_system>(std::ref(*this));
}

entity_id cosmos::substantialize(aggregate_id aggregate) {
	entity_id new_id;
	new_id.aggregate_id::operator=(aggregate);

	lists_of_processing_subjects.add_entity_to_matching_lists(new_id);

	messages::new_entity_message notification;
	notification.subject = new_id;
	messages.post(notification);

	return new_id;
}

void cosmos::reserve_storage_for_aggregates(size_t n) {
	components_and_aggregates.reserve_storage_for_aggregates(n);
}

entity_id cosmos::clone_entity(entity_id e) {
	return substantialize(components_and_aggregates.clone_aggregate(e));
}

void cosmos::delete_entity(entity_id e) {
	ensure(e.alive());
	lists_of_processing_subjects.remove_entity_from_lists(e);
	components_and_aggregates.free_aggregate(e);
}

size_t cosmos::entities_count() const {
	return components_and_aggregates.aggregates_count();
}