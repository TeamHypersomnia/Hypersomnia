#include "cosmos.h"

#include "game/systems_stateless/movement_system.h"
#include "game/systems_stateless/visibility_system.h"
#include "game/systems_stateless/pathfinding_system.h"
#include "game/systems_stateless/animation_system.h"
#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/input_system.h"
#include "game/systems_stateless/gun_system.h"
#include "game/systems_stateless/crosshair_system.h"
#include "game/systems_stateless/rotation_copying_system.h"
#include "game/systems_stateless/position_copying_system.h"
#include "game/systems_stateless/damage_system.h"
#include "game/systems_stateless/destroy_system.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/behaviour_tree_system.h"
#include "game/systems_stateless/car_system.h"
#include "game/systems_stateless/driver_system.h"
#include "game/systems_stateless/trigger_detector_system.h"
#include "game/systems_stateless/item_system.h"
#include "game/systems_stateless/force_joint_system.h"
#include "game/systems_stateless/intent_contextualization_system.h"
#include "game/systems_stateless/trace_system.h"
#include "game/systems_stateless/melee_system.h"
#include "game/systems_stateless/sentience_system.h"
#include "game/systems_stateless/destruction_system.h"
#include "game/systems_stateless/sound_existence_system.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmic_profiler.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/detail/item_slot_transfer_request.h"

#include "game/messages/health_event.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/inventory_utils.h"

void cosmos::complete_resubstantiation() {
	profiler.complete_resubstantiation.new_measurement();
	
	destroy_substance_completely();
	create_substance_completely();

	profiler.complete_resubstantiation.end_measurement();
}

void cosmos::destroy_substance_completely() {
	systems_temporary.~storage_for_all_systems_temporary();
	new (&systems_temporary) storage_for_all_systems_temporary;

	size_t n = significant.pool_for_aggregates.capacity();

	systems_temporary.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos::create_substance_completely() {
	for (auto& ordered_pair : guid_map_for_transport) {
		create_substance_for_entity(get_handle(ordered_pair.second));
	}

	//for (auto it = guid_map_for_transport.rbegin(); it != guid_map_for_transport.rend(); ++it) {
	//	create_substance_for_entity(get_handle((*it).second));
	//}
}

void cosmos::destroy_substance_for_entity(const const_entity_handle h) {
	auto destructor = [h](auto& sys) {
		sys.destruct(h);
	};

	systems_temporary.for_each(destructor);
}

void cosmos::create_substance_for_entity(const const_entity_handle h) {
	if (h.has<components::substance>()) {
		auto constructor = [h](auto& sys) {
			sys.construct(h);
		};

		systems_temporary.for_each(constructor);
	}
}

cosmos::cosmos(const unsigned reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
	entity_debug_names[0] = "dead entity";
}

const std::string& cosmos::get_debug_name(entity_id id) const {
	return entity_debug_names[id.pool.indirection_index + 1];
}

void cosmos::set_debug_name(const entity_id id, const std::string& new_debug_name) {
	entity_debug_names[id.pool.indirection_index + 1] = new_debug_name;
}

void cosmos::delete_debug_name(const entity_id id) {
	entity_debug_names[id.pool.indirection_index + 1] = "dead entity";
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

cosmos& cosmos::operator=(const significant_state& b) {
	profiler.duplication.new_measurement();
	significant = b;
	profiler.duplication.end_measurement();
	refresh_for_new_significant_state();
	return *this;
}

cosmos& cosmos::operator=(const cosmos& b) {
	b.profiler.duplication.new_measurement();
	profiler.duplication.new_measurement();
	significant = b.significant;
#if COSMOS_TRACKS_GUIDS
	guid_map_for_transport = b.guid_map_for_transport;
#endif
	profiler.duplication.end_measurement();
	b.profiler.duplication.end_measurement();
	
	//complete_resubstantiation();
	profiler.complete_resubstantiation.new_measurement();
	systems_temporary = b.systems_temporary;
	//reserve_storage_for_entities(get_maximum_entities());
	profiler.complete_resubstantiation.end_measurement();
	return *this;
}

#if COSMOS_TRACKS_GUIDS

void cosmos::assign_next_guid(const entity_handle new_entity) {
	auto this_guid = significant.meta.next_entity_guid++;

	guid_map_for_transport[this_guid] = new_entity;
	new_entity.get<components::guid>().value = this_guid;
}

void cosmos::clear_guid(const entity_handle cleared) {
	guid_map_for_transport.erase(get_guid(cleared));
}

void cosmos::remap_guids() {
	guid_map_for_transport.clear();

	for_each_entity_id([this](const entity_id id) {
		guid_map_for_transport[get_guid(get_handle(id))] = id;
	});
}

#endif

void cosmos::refresh_for_new_significant_state() {
#if COSMOS_TRACKS_GUIDS
	remap_guids();
#endif
	complete_resubstantiation();
}

void cosmos::complete_resubstantiation(const const_entity_handle h) {
	destroy_substance_for_entity(h);
	create_substance_for_entity(h);
}

void cosmos::reserve_storage_for_entities(const size_t n) {
	get_aggregate_pool().initialize_space(n);
	reserve_storage_for_all_components(n);
	entity_debug_names.resize(n + 1);

	auto reservation_lambda = [n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	};

	systems_temporary.for_each(reservation_lambda);
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", entities_count());
}

std::vector<entity_handle> cosmos::get(const processing_subjects list) {
	return systems_temporary.get<processing_lists_system>().get(list, *this);
}

std::vector<const_entity_handle> cosmos::get(const processing_subjects list) const {
	return systems_temporary.get<processing_lists_system>().get(list, *this);
}

size_t cosmos::get_rng_seed_for(const entity_id id) const {
	size_t transform_hash = 0;
	const auto tr = get_handle(id).get_logic_transform();
	transform_hash = static_cast<size_t>(std::abs(tr.pos.x)*100.0);
	transform_hash += static_cast<size_t>(std::abs(tr.pos.y)*100.0);
	transform_hash += static_cast<size_t>(std::abs(tr.rotation)*100.0);

	return get_handle(id).get_guid() + transform_hash;
}

float cosmos::get_total_time_passed_in_seconds(const float view_interpolation_ratio) const {
	return (significant.meta.total_steps_passed + view_interpolation_ratio) * significant.meta.delta.in_seconds();
}

float cosmos::get_total_time_passed_in_seconds() const {
	return significant.meta.total_steps_passed * significant.meta.delta.in_seconds();
}

unsigned cosmos::get_total_steps_passed() const {
	return significant.meta.total_steps_passed;
}

augs::stepped_timestamp cosmos::get_timestamp() const {
	augs::stepped_timestamp result;
	result.step = significant.meta.total_steps_passed;
	return result;
}

const augs::delta& cosmos::get_fixed_delta() const {
	return significant.meta.delta;
}

void cosmos::set_fixed_delta(const augs::delta& dt) {
	significant.meta.delta = dt;
}

void cosmos::set_fixed_delta(const unsigned steps_per_second) {
	significant.meta.delta = static_cast<float>(1000.f / steps_per_second);
}

entity_handle cosmos::allocate_new_entity() {
	auto raw_pool_id = get_aggregate_pool().allocate();

	return entity_handle(*this, raw_pool_id);
}

entity_handle cosmos::create_entity(const std::string& debug_name) {
	auto new_entity = allocate_new_entity();
	set_debug_name(new_entity, debug_name);
	new_entity += components::guid();

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif
	//ensure(new_entity.get_id().pool.indirection_index != 37046);
	//ensure(new_entity.get_id().pool.indirection_index != 36985);
	return new_entity;
}

#if COSMOS_TRACKS_GUIDS
entity_handle cosmos::create_entity_with_specific_guid(const std::string& debug_name, const entity_guid specific_guid) {
	const auto new_entity = allocate_new_entity();
	new_entity += components::guid();

	guid_map_for_transport[specific_guid] = new_entity;
	new_entity.get<components::guid>().value = specific_guid;
	return new_entity;
}
#endif

entity_handle cosmos::clone_entity(const entity_id copied_entity_id) {
	entity_handle copied_entity = get_handle(copied_entity_id);
	
	if (copied_entity.dead()) {
		return get_handle(entity_id());
	}

	auto new_entity = allocate_new_entity();
	clone_all_components<components::substance>(copied_entity, new_entity);
	set_debug_name(new_entity, "+" + copied_entity.get_debug_name());

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif

	// zero-out non-trivial relational components

	for_each_type<
		components::child, 
		components::physical_relations, 
		components::sub_entities
	>([copied_entity, new_entity](auto c) {
		typedef decltype(c) T;
		
		if (copied_entity.has<T>()) {
			new_entity.get<T>() = T();
		}
	});

	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}

	new_entity.make_cloned_sub_entities_recursive(copied_entity_id);
	
	if (copied_entity.get_owner_body() == copied_entity) {
		new_entity.set_owner_body(new_entity);
	}

	if (copied_entity.has<components::substance>()) {
		new_entity.add(components::substance());
	}

	return new_entity;
}

void cosmos::delete_entity(const entity_id e) {
	const auto handle = get_handle(e);
	if (handle.dead()) {
		LOG("Warning! Attempt to delete a dead entity: %x", handle);
		return;
	}

	//ensure(handle.alive());

	const bool should_destruct_now_to_avoid_repeated_resubstantiation = handle.has<components::substance>();

	if (should_destruct_now_to_avoid_repeated_resubstantiation) {
		handle.remove<components::substance>();
	}

#if COSMOS_TRACKS_GUIDS
	clear_guid(handle);
#endif
	// now manipulation of substanceless entity won't trigger redundant resubstantiation

	const auto owner_body = handle.get_owner_body();

	const bool should_release_dependency = owner_body != handle;

	if (should_release_dependency) {
		handle.set_owner_body(handle);
	}

	remove_all_components(get_handle(e));
	get_aggregate_pool().free(e);
	delete_debug_name(e);
}

void cosmos::advance_deterministic_schemata(const cosmic_entropy& input) {
	data_living_one_step transient;
	logic_step step(*this, input, transient);

	advance_deterministic_schemata_and_queue_destructions(step);
	perform_deletions(step);
}

void cosmos::advance_deterministic_schemata_and_queue_destructions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	auto& performance = profiler;
	
	performance.start(meter_type::LOGIC);

	physics_system::contact_listener listener(step.cosm);
	
	perform_transfers(step.entropy.transfer_requests, step);
	
	performance.entropy_length.measure(step.entropy.length());

	sentience_system().cast_spells(step);

	input_system().make_intent_messages(step);

	intent_contextualization_system().contextualize_crosshair_action_intents(step);
	intent_contextualization_system().contextualize_use_button_intents(step);
	intent_contextualization_system().contextualize_movement_intents(step);

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

	performance.start(meter_type::PHYSICS);
	listener.during_step = true;
	systems_temporary.get<physics_system>().step_and_set_new_transforms(step);
	listener.during_step = false;
	performance.stop(meter_type::PHYSICS);
	rotation_copying_system().update_rotations(step.cosm);
	position_copying_system().update_transforms(step);

	//particles_simulation_system().create_particle_effects(step);

	trace_system().lengthen_sprites_of_traces(step);

	crosshair_system().generate_crosshair_intents(step);
	crosshair_system().apply_crosshair_intents_to_base_offsets(step);
	crosshair_system().apply_base_offsets_to_crosshair_transforms(step);

	trigger_detector_system().send_trigger_confirmations(step);

//	item_system().translate_gui_intents_to_transfer_requests(step);
	item_system().handle_trigger_confirmations_as_pick_requests(step);
	item_system().handle_throw_item_intents(step);

	damage_system().destroy_outdated_bullets(step);
	damage_system().destroy_colliding_bullets_and_send_damage(step);

	destruction_system().generate_damages_from_forceful_collisions(step);
	destruction_system().apply_damages_and_split_fixtures(step);

	sentience_system().regenerate_values_and_advance_spell_logic(step);
	sentience_system().apply_damage_and_generate_health_events(step);
	systems_temporary.get<physics_system>().post_and_clear_accumulated_collision_messages(step);
	sentience_system().cooldown_aimpunches(step);
	sentience_system().set_borders(step);

	driver_system().release_drivers_due_to_requests(step);
	driver_system().assign_drivers_from_successful_trigger_hits(step);
	driver_system().release_drivers_due_to_ending_contact_with_wheel(step);

	particles_existence_system().game_responses_to_particle_effects(step);
	particles_existence_system().create_particle_effects(step);

	sound_existence_system().game_responses_to_sound_effects(step);
	// gui_system().translate_game_events_for_hud(step);

	performance.start(meter_type::VISIBILITY);
	visibility_system().respond_to_visibility_information_requests(step);
	performance.stop(meter_type::VISIBILITY);

	performance.start(meter_type::AI);
	behaviour_tree_system().evaluate_trees(step);
	performance.stop(meter_type::AI);

	performance.start(meter_type::PATHFINDING);
	pathfinding_system().advance_pathfinding_sessions(step);
	performance.stop(meter_type::PATHFINDING);

	auto& transfers = step.transient.messages.get_queue<item_slot_transfer_request_data>();
	perform_transfers(transfers, step);

	particles_existence_system().destroy_dead_streams(step);
	sound_existence_system().destroy_dead_sounds(step);

	trace_system().destroy_outdated_traces(step);

	destroy_system().queue_children_of_queued_entities(step);

	trace_system().spawn_finishing_traces_for_destroyed_objects(step);

	const bool has_no_destruction_callback_queued_any_additional_destruction = step.transient.messages.get_queue<messages::queue_destruction>().empty();
	ensure(has_no_destruction_callback_queued_any_additional_destruction);

	listener.~contact_listener();

	movement_system().generate_movement_responses(step);

	animation_system().game_responses_to_animation_messages(step);

	animation_system().handle_animation_messages(step);
	animation_system().progress_animation_states(step);

	//position_copying_system().update_transforms(step);
	//rotation_copying_system().update_rotations(step.cosm);

	profiler.raycasts.measure(systems_temporary.get<physics_system>().ray_casts_since_last_step);

	++significant.meta.total_steps_passed;

	performance.stop(meter_type::LOGIC);
}

void cosmos::perform_deletions(const logic_step step) {
	destroy_system().perform_deletions(step);
}
