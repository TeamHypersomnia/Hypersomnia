#include "cosmos.h"

#include "game/temporary_systems/physics_system.h"
#include "game/temporary_systems/dynamic_tree_system.h"
#include "game/systematic/movement_system.h"
#include "game/systematic/visibility_system.h"
#include "game/systematic/pathfinding_system.h"
#include "game/systematic/animation_system.h"
#include "game/systematic/render_system.h"
#include "game/systematic/input_system.h"
#include "game/systematic/gun_system.h"
#include "game/systematic/crosshair_system.h"
#include "game/systematic/rotation_copying_system.h"
#include "game/systematic/position_copying_system.h"
#include "game/systematic/damage_system.h"
#include "game/systematic/destroy_system.h"
#include "game/systematic/particles_system.h"
#include "game/systematic/behaviour_tree_system.h"
#include "game/systematic/car_system.h"
#include "game/systematic/driver_system.h"
#include "game/systematic/trigger_detector_system.h"
#include "game/systematic/item_system.h"
#include "game/systematic/force_joint_system.h"
#include "game/systematic/intent_contextualization_system.h"
#include "game/systematic/gui_system.h"
#include "game/systematic/trace_system.h"
#include "game/systematic/melee_system.h"
#include "game/systematic/sentience_system.h"

#include "game/enums/render_layer.h"

#include "cosmos.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmic_profiler.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"

#include "game/messages/health_event.h"

#include "game/transcendental/entity_relations.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include <sstream>

void cosmos::complete_resubstantialization() {
	profiler.complete_resubstantiation.new_measurement();

	temporary_systems.~storage_for_all_temporary_systems();
	new (&temporary_systems) storage_for_all_temporary_systems;

	size_t n = significant.pool_for_aggregates.capacity();

	temporary_systems.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});

	significant.pool_for_aggregates.for_each_id([this](entity_id id) {
		auto h = get_handle(id);
		complete_resubstantialization(h);
	});

	profiler.complete_resubstantiation.end_measurement();
}

cosmos::cosmos() {

}

cosmos::cosmos(const cosmos& b) {
	*this = b;
}

bool cosmos::operator==(const cosmos& b) const {
	return significant == b.significant;
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}

cosmos& cosmos::operator=(const cosmos& b) {
	operator=(b.significant);
	return *this;
}

cosmos& cosmos::operator=(const significant_state& b) {
	significant = b;
	complete_resubstantialization();
	return *this;
}

void cosmos::complete_resubstantialization(entity_handle h) {
	temporary_systems.for_each([h](auto& sys) {
		sys.destruct(h);
	});

	if (h.has<components::substance>()) {
		temporary_systems.for_each([h](auto& sys) {
			sys.construct(h);
		});
	}
}

void cosmos::reserve_storage_for_entities(size_t n) {
	reserve_storage_for_aggregates(n);

	temporary_systems.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", entities_count());
}

std::vector<entity_handle> cosmos::get(processing_subjects list) {
	return temporary_systems.get<processing_lists_system>().get(list, *this);
}

std::vector<const_entity_handle> cosmos::get(processing_subjects list) const {
	return temporary_systems.get<processing_lists_system>().get(list, *this);
}

randomization cosmos::get_rng_for(entity_id id) const {
	return { std::abs(id.version) + std::abs(id.indirection_index) + static_cast<size_t>(significant.delta.get_total_steps_passed()) };
}

entity_handle cosmos::get_handle(entity_id id) {
	return entity_handle(*this, id);
}

const_entity_handle cosmos::get_handle(entity_id id) const {
	return const_entity_handle(*this, id);
}

inventory_slot_handle cosmos::get_handle(inventory_slot_id id) {
	return inventory_slot_handle(*this, id);
}

const_inventory_slot_handle cosmos::get_handle(inventory_slot_id id) const {
	return const_inventory_slot_handle(*this, id);
}

entity_handle cosmos::create_entity(std::string debug_name) {
	return get_handle(allocate_aggregate(debug_name));
}

entity_handle cosmos::clone_entity(entity_id e) {
	const_entity_handle copied_entity = get_handle(e);
	
	if (copied_entity.dead())
		return get_handle(entity_id());

	auto new_entity = get_handle(clone_aggregate(copied_entity));

	new_entity.make_cloned_sub_entities_recursive(e);
	new_entity.assign_associated_entities(e);
	
	if (copied_entity.get_owner_body() == copied_entity) {
		new_entity.set_owner_body(new_entity);
	}

	return new_entity;
}

void cosmos::delete_entity(entity_id e) {
	auto handle = get_handle(e);
	if (handle.dead()) {
		LOG("Warning! Attempt to delete a dead entity: %x", e);
		return;
	}
	//ensure(handle.alive());

	bool should_destruct_now_to_avoid_repeated_resubstantialization = handle.has<components::substance>();

	if (should_destruct_now_to_avoid_repeated_resubstantialization)
		handle.remove<components::substance>();

	// now manipulation of substanceless entity won't trigger redundant resubstantialization

	auto owner_body = handle.get_owner_body();

	bool should_release_dependency = owner_body != handle;

	if (should_release_dependency) {
		handle.set_owner_body(owner_body);
	}

	free_aggregate(e);
}

size_t cosmos::entities_count() const {
	return aggregates_count();
}

size_t cosmos::get_maximum_entities() const {
	return significant.pool_for_aggregates.capacity();
}

void cosmos::advance_deterministic_schemata(cosmic_entropy input, fixed_callback pre_solve, fixed_callback post_solve) {
	fixed_step step(*this, input);
	
	if (pre_solve)
		pre_solve(step);

	advance_deterministic_schemata(step);

	if (post_solve)
		post_solve(step);
}

void cosmos::advance_deterministic_schemata(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& performance = profiler;

	profiler.entropy_length.measure(step.entropy.length());

	//gui_system().advance_gui_elements();
	//gui_system().translate_raw_window_inputs_to_gui_events();
	//gui_system().suppress_inputs_meant_for_gui();
	//gui_system().switch_to_gui_mode_and_back();

	performance.start(meter_type::INTERPOLATION);
	render_system().set_current_transforms_as_previous_for_interpolation(cosmos);
	performance.stop(meter_type::INTERPOLATION);

	performance.start(meter_type::LOGIC);
	
	input_system().make_intents_from_raw_entropy(step);

	intent_contextualization_system().contextualize_crosshair_action_intents(step);
	intent_contextualization_system().contextualize_use_button_intents(step);
	intent_contextualization_system().contextualize_movement_intents(step);

	driver_system().release_drivers_due_to_requests(step);

	movement_system().set_movement_flags_from_input(step);
	movement_system().apply_movement_forces(step.cosm);

	gun_system().consume_gun_intents(step);
	gun_system().launch_shots_due_to_pressed_triggers(step);

	car_system().set_steering_flags_from_intents(step);
	car_system().apply_movement_forces(step);

	melee_system().consume_melee_intents(step);
	melee_system().initiate_and_update_moves(step);

	trigger_detector_system().consume_trigger_detector_presses(step);
	trigger_detector_system().post_trigger_requests_from_continuous_detectors(step);

	force_joint_system().apply_forces_towards_target_entities(step);

	rotation_copying_system().update_physical_motors(step.cosm);
	performance.start(meter_type::PHYSICS);
	temporary_systems.get<physics_system>().step_and_set_new_transforms(step);
	performance.stop(meter_type::PHYSICS);
	position_copying_system().update_transforms(step);

	particles_system().create_particle_effects(step);

	trace_system().lengthen_sprites_of_traces(step);

	crosshair_system().generate_crosshair_intents(step);
	crosshair_system().apply_crosshair_intents_to_base_offsets(step);
	crosshair_system().apply_base_offsets_to_crosshair_transforms(step);

	trigger_detector_system().send_trigger_confirmations(step);

//	item_system().translate_gui_intents_to_transfer_requests(step);
	item_system().handle_trigger_confirmations_as_pick_requests(step);
	item_system().handle_holster_item_intents(step);
	item_system().handle_throw_item_intents(step);

	driver_system().assign_drivers_from_successful_trigger_hits(step);
	driver_system().release_drivers_due_to_ending_contact_with_wheel(step);

	damage_system().destroy_outdated_bullets(step);
	damage_system().destroy_colliding_bullets_and_send_damage(step);

	sentience_system().apply_damage_and_generate_health_events(step);
	sentience_system().cooldown_aimpunches(step);

	particles_system().game_responses_to_particle_effects(step);
	particles_system().create_particle_effects(step);

	// gui_system().translate_game_events_for_hud(step);

	performance.start(meter_type::VISIBILITY);
	visibility_system().generate_visibility_and_sight_information(step.cosm);
	performance.stop(meter_type::VISIBILITY);

	performance.start(meter_type::AI);
	behaviour_tree_system().evaluate_trees(step);
	performance.stop(meter_type::AI);

	performance.start(meter_type::PATHFINDING);
	pathfinding_system().advance_pathfinding_sessions(step.cosm);
	performance.stop(meter_type::PATHFINDING);

	particles_system().step_streams_and_particles(step);
	particles_system().destroy_dead_streams(step);
	trace_system().destroy_outdated_traces(step);

	destroy_system().queue_children_of_queued_entities(step);

	trace_system().spawn_finishing_traces_for_destroyed_objects(step);

	bool has_no_destruction_callback_queued_any_additional_destruction = step.messages.get_queue<messages::queue_destruction>().empty();
	ensure(has_no_destruction_callback_queued_any_additional_destruction);

	destroy_system().perform_deletions(step);

	movement_system().generate_movement_responses(step);

	animation_system().game_responses_to_animation_messages(step);

	animation_system().handle_animation_messages(step);
	animation_system().progress_animation_states(step);

	performance.start(meter_type::RENDERING);

	position_copying_system().update_transforms(step);
	rotation_copying_system().update_rotations(step.cosm);

	profiler.raycasts.measure(temporary_systems.get<physics_system>().ray_casts_since_last_step);
	performance.stop(meter_type::LOGIC);
}