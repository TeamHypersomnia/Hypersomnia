#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "game/cosmos/solvers/standard_solver.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmic_functions.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/physics/contact_listener.h"

#include "game/stateless_systems/movement_system.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/stateless_systems/pathfinding_system.h"
#include "game/stateless_systems/input_system.h"
#include "game/stateless_systems/gun_system.h"
#include "game/stateless_systems/crosshair_system.h"
#include "game/stateless_systems/missile_system.h"
#include "game/stateless_systems/destroy_system.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/behaviour_tree_system.h"
#include "game/stateless_systems/car_system.h"
#include "game/stateless_systems/driver_system.h"
#include "game/stateless_systems/item_system.h"
#include "game/stateless_systems/force_joint_system.h"
#include "game/stateless_systems/intent_contextualization_system.h"
#include "game/stateless_systems/trace_system.h"
#include "game/stateless_systems/melee_system.h"
#include "game/stateless_systems/sentience_system.h"
#include "game/stateless_systems/destruction_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/stateless_systems/demolitions_system.h"
#include "game/stateless_systems/physics_system.h"
#include "game/stateless_systems/movement_path_system.h"
#include "game/stateless_systems/animation_system.h"
#include "game/stateless_systems/remnant_system.h"

void standard_solve(const logic_step step) {
	auto& cosm = step.get_cosmos();
	auto& performance = cosm.profiler;
	auto& global = cosm.get_global_solvable();

	auto logic_scope = measure_scope(performance.logic);

	auto total_raycasts_scope = cosm.measure_raycasts(performance.total_step_raycasts);

	contact_listener listener(cosm);

	for (const auto& p : step.get_entropy().players) {
		perform_transfers(p.second.transfers, step);
	}

	performance.entropy_length.measure(step.get_entropy().length());

	sentience_system().cast_spells(step);

	input_system().make_input_messages(step);

	intent_contextualization_system().contextualize_crosshair_action_intents(step);
	intent_contextualization_system().contextualize_movement_intents(step);

	intent_contextualization_system().handle_use_button_presses(step);
	intent_contextualization_system().advance_use_button(step);

	{
		auto scope = measure_scope(performance.movement_paths);
		movement_path_system().advance_paths(step);
	}

	{
		auto scope = measure_scope(performance.stateful_animations);
		animation_system().advance_stateful_animations(step);
	}

	movement_system().set_movement_flags_from_input(step);
	movement_system().apply_movement_forces(step);

	crosshair_system().handle_crosshair_intents(step);
	crosshair_system().update_base_offsets(step);
	sentience_system().rotate_towards_crosshairs_and_driven_vehicles(step);

	gun_system().consume_gun_intents(step);
	gun_system().launch_shots_due_to_pressed_triggers(step);

	car_system().set_steering_flags_from_intents(step);
	car_system().apply_movement_forces(step);

	melee_system().consume_melee_intents(step);
	melee_system().initiate_and_update_moves(step);

	force_joint_system().apply_forces_towards_target_entities(step);
	item_system().handle_throw_item_intents(step);
	item_system().handle_reload_intents(step);
	item_system().advance_reloading_contexts(step);
	global.solve_item_mounting(step);
	item_system().handle_wielding_requests(step);
	demolitions_system().detonate_fuses(step);
	demolitions_system().advance_cascade_explosions(step);

	{
		listener.during_step = true;
		physics_system().step_and_set_new_transforms(step);
		listener.during_step = false;
	}

	trace_system().lengthen_sprites_of_traces(step);

	crosshair_system().integrate_crosshair_recoils(step);

	item_system().pick_up_touching_items(step);

	missile_system().ricochet_missiles(step);
	missile_system().detonate_colliding_missiles(step);
	missile_system().detonate_expired_missiles(step);

	destruction_system().generate_damages_from_forceful_collisions(step);
	destruction_system().apply_damages_and_split_fixtures(step);

	sentience_system().process_special_results_of_health_events(step);
	sentience_system().regenerate_values_and_advance_spell_logic(step);
	sentience_system().apply_damage_and_generate_health_events(step);
	physics_system().post_and_clear_accumulated_collision_messages(step);
	sentience_system().cooldown_aimpunches(step);

	driver_system().release_drivers_due_to_requests(step);
	driver_system().assign_drivers_who_touch_wheels(step);
	driver_system().release_drivers_due_to_ending_contact_with_wheel(step);

	particles_existence_system().play_particles_from_events(step);
	particles_existence_system().displace_streams(step);
	sound_existence_system().play_sounds_from_events(step);

	{
		auto scope = measure_scope(performance.visibility);
		auto visibility_raycasts_scope = cosm.measure_raycasts(performance.visibility_raycasts);

		visibility_system(DEBUG_LOGIC_STEP_LINES).calc_visibility(step);
	}

	{
		auto scope = measure_scope(performance.ai);
		behaviour_tree_system().evaluate_trees(step);
	}

	{
		auto pathfinding_raycasts_scope = cosm.measure_raycasts(performance.pathfinding_raycasts);

#if TODO_PATHFINDING
		auto scope = measure_scope(performance.pathfinding);
		pathfinding_system().advance_pathfinding_sessions(step);
#endif
	}

	{
		auto& transfers = step.get_queue<item_slot_transfer_request>();
		perform_transfers(transfers, step);
	}

	trace_system().destroy_outdated_traces(step);
	remnant_system().shrink_and_destroy_remnants(step);

	const auto queued_before_marking_num = step.get_queue<messages::queue_deletion>().size();

	destroy_system().mark_queued_entities_and_their_children_for_deletion(step);

	trace_system().spawn_finishing_traces_for_deleted_entities(step);

	listener.~contact_listener();

	const auto queued_at_end_num = step.get_queue<messages::queue_deletion>().size();

	ensure_eq(queued_at_end_num, queued_before_marking_num);
}
