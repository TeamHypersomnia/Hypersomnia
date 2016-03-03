#include "game_world.h"

#include "game_framework/all_component_includes.h"
#include "game_framework/all_system_includes.h"
#include "game_framework/all_message_includes.h"

#include "game_framework/globals/layers.h"

#include <functional>

using namespace components;
using namespace messages;

game_world::game_world(augs::overworld& parent_overworld) : world(parent_overworld) {
	register_types_of_messages_components_systems();
}

void game_world::register_types_of_messages_components_systems() {
	register_component<animation>();
	register_component<animation_response>();
	register_component<behaviour_tree>();
	register_component<camera>();
	register_component<position_copying>();
	register_component<crosshair>();
	register_component<damage>();
	register_component<gun>();
	register_component<input_receiver>();
	register_component<rotation_copying>();
	register_component<movement>();
	register_component<particle_emitter>();
	register_component<particle_group>();
	register_component<pathfinding>();
	register_component<physics>();
	register_component<render>();
	register_component<steering>();
	register_component<transform>();
	register_component<visibility>();
	register_component<sprite>();
	register_component<polygon>();
	register_component<tile_layer>();
	register_component<car>();
	register_component<driver>();
	register_component<trigger>();
	register_component<trigger_query_detector>();
	register_component<fixtures>();
	register_component<item>();
	register_component<container>();
	register_component<force_joint>();
	register_component<physics_definition>();
	register_component<item_slot_transfers>();
	register_component<melee>();
	register_component<gui_element>();
	register_component<trigger_collision_detector>();
	
	register_system<input_system>();
	register_system<steering_system>();
	register_system<movement_system>();
	register_system<animation_system>();
	register_system<crosshair_system>();
	register_system<rotation_copying_system>();
	register_system<physics_system>();
	register_system<visibility_system>();
	register_system<pathfinding_system>();
	register_system<gun_system>();
	register_system<particle_group_system>();
	register_system<particle_emitter_system>();
	register_system<render_system>();
	register_system<camera_system>();
	register_system<position_copying_system>();
	register_system<damage_system>();
	register_system<destroy_system>();
	register_system<behaviour_tree_system>();
	register_system<car_system>();
	register_system<driver_system>();
	register_system<trigger_detector_system>();
	register_system<item_system>();
	register_system<force_joint_system>();
	register_system<intent_contextualization_system>();
	register_system<gui_system>();

	register_message_queue<intent_message>();
	register_message_queue<damage_message>();
	register_message_queue<destroy_message>();
	register_message_queue<animation_message>();
	register_message_queue<animation_response_message>();
	register_message_queue<collision_message>();
	register_message_queue<particle_burst_message>();
	register_message_queue<shot_message>();
	register_message_queue<raw_window_input_message>();
	register_message_queue<unmapped_intent_message>();
	register_message_queue<crosshair_intent_message>();
	register_message_queue<trigger_hit_confirmation_message>();
	register_message_queue<trigger_hit_request_message>();
	register_message_queue<new_entity_message>();
	register_message_queue<camera_render_request_message>();
	register_message_queue<item_slot_transfer_intent>();
	register_message_queue<item_slot_transfer_request>();

	register_message_callback<item_slot_transfer_intent>(std::bind(&item_system::constrain_item_slot_transfer_intents, &get_system<item_system>()));
	register_message_callback<item_slot_transfer_request>(std::bind(&item_system::consume_item_slot_transfer_requests, &get_system<item_system>()));
}

void game_world::call_drawing_time_systems() {
	get_system<render_system>().determine_visible_entities_from_every_camera();

	get_system<render_system>().calculate_and_set_interpolated_transforms();
	
	/* read-only message generation */

	get_system<animation_system>().transform_response_requests_to_animation_messages();
	
	get_system<gui_system>().translate_raw_window_inputs_to_gui_events();
	
	get_system<input_system>().post_unmapped_intents_from_raw_window_inputs();
	get_system<gui_system>().suppress_inputs_meant_for_gui();
	get_system<input_system>().map_unmapped_intents_to_entities();

	/* note that this call cannot be resimulated for it is an incremental operation */
	get_system<crosshair_system>().generate_crosshair_intents();

	/* we do not wait for the the logic step with applying crosshair's base offset because we want an instant visual feedback.
	the logic will resimulate these crosshair intents deterministically. See perform_logic_step, the same two calls are made.
	*/

	get_system<crosshair_system>().apply_crosshair_intents_to_base_offsets();
	get_system<crosshair_system>().apply_base_offsets_to_crosshair_transforms();

	/* application of messages */
	get_system<animation_system>().handle_animation_messages();
	get_system<animation_system>().progress_animation_states();

	get_system<crosshair_system>().animate_crosshair_sizes();
	get_system<movement_system>().animate_movement();

	get_system<position_copying_system>().update_transforms();
	get_system<camera_system>().resolve_cameras_transforms_and_smoothing();
	get_system<rotation_copying_system>().update_rotations();

	get_system<camera_system>().post_render_requests_for_all_cameras();

	get_system<input_system>().acquire_new_events_posted_by_drawing_time_systems();

	get_system<gui_system>().rebuild_gui_tree_based_on_game_state();
}

void game_world::restore_transforms_after_drawing() {
	get_system<render_system>().restore_actual_transforms();
}

void game_world::perform_logic_step() {
	get_system<input_system>().post_all_events_posted_by_drawing_time_systems_since_last_step();
	
	get_system<gui_system>().switch_to_gui_mode_and_back();

	get_system<input_system>().map_unmapped_intents_to_entities();

	/* begin receivers of new_entity messages changes */

	get_system<render_system>().add_entities_to_rendering_tree();
	get_system<physics_system>().create_bodies_and_fixtures_from_physics_definitions();

	get_message_queue<new_entity_message>().clear();
	/* end receivers of new_entity messages */

	get_system<crosshair_system>().apply_crosshair_intents_to_base_offsets();
	get_system<crosshair_system>().apply_base_offsets_to_crosshair_transforms();

	get_system<camera_system>().react_to_input_intents();

	/* intent delegation stage (various ownership relations) */
	get_system<intent_contextualization_system>().contextualize_use_button_intents();
	get_system<intent_contextualization_system>().contextualize_crosshair_action_intents();
	/* end of intent delegation stage */

	get_system<driver_system>().release_drivers_due_to_requests();

	get_system<trigger_detector_system>().consume_trigger_detector_presses();
	get_system<trigger_detector_system>().post_trigger_requests_from_continuous_detectors();
	get_system<trigger_detector_system>().send_trigger_confirmations();

	get_system<item_system>().handle_trigger_confirmations_as_pick_requests();
	get_system<item_system>().handle_holster_item_intents();
	get_system<item_system>().handle_throw_item_intents();

	get_system<driver_system>().assign_drivers_from_triggers();
	get_system<driver_system>().release_drivers_due_to_ending_contact_with_wheel();

	get_system<intent_contextualization_system>().contextualize_movement_intents();

	get_system<render_system>().remove_entities_from_rendering_tree();
	get_system<physics_system>().destroy_fixtures_and_bodies();
	get_system<destroy_system>().delete_queued_entities();

	get_system<force_joint_system>().apply_forces_towards_target_entities();

	get_system<car_system>().set_steering_flags_from_intents();
	get_system<car_system>().apply_movement_forces();

	get_system<movement_system>().set_movement_flags_from_input();
	get_system<movement_system>().apply_movement_forces();

	get_system<rotation_copying_system>().update_physical_motors();
	get_system<physics_system>().step_and_set_new_transforms();
	get_system<position_copying_system>().update_transforms();

	get_system<render_system>().set_current_transforms_as_previous_for_interpolation();

	++current_step_number;
}