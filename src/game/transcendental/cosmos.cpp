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
#include "game/systems_stateless/item_system.h"
#include "game/systems_stateless/force_joint_system.h"
#include "game/systems_stateless/intent_contextualization_system.h"
#include "game/systems_stateless/trace_system.h"
#include "game/systems_stateless/melee_system.h"
#include "game/systems_stateless/sentience_system.h"
#include "game/systems_stateless/destruction_system.h"
#include "game/systems_stateless/sound_existence_system.h"
#include "game/systems_stateless/hand_fuse_system.h"

#include "game/enums/render_layer.h"

#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_profiler.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/spell_logic.h"

#include "game/messages/health_event.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/test_scenes/resource_setups/all.h"

#include "game/components/fixtures_component.h"

void cosmos::complete_reinference() {
	profiler.complete_reinference.new_measurement();
	
	destroy_inferred_state_completely();
	create_inferred_state_completely();

	profiler.complete_reinference.end_measurement();
}

void cosmos::destroy_inferred_state_completely() {
	systems_inferred.~storage_for_all_systems_inferred();
	new (&systems_inferred) storage_for_all_systems_inferred;

	const auto n = significant.pool_for_aggregates.capacity();

	systems_inferred.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos::create_inferred_state_completely() {
	for (const auto& ordered_pair : guid_map_for_transport) {
		create_inferred_state_for(get_handle(ordered_pair.second));
	}

	//for (auto it = guid_map_for_transport.rbegin(); it != guid_map_for_transport.rend(); ++it) {
	//	create_inferred_state_for(get_handle((*it).second));
	//}
}

void cosmos::destroy_inferred_state_of(const const_entity_handle h) {
	auto destructor = [h](auto& sys) {
		sys.destroy_inferred_state_of(h);
	};

	systems_inferred.for_each(destructor);
}

void cosmos::create_inferred_state_for(const const_entity_handle h) {
	if (h.is_inferred_state_activated()) {
		auto constructor = [h](auto& sys) {
			sys.create_inferred_state_for(h);
		};

		systems_inferred.for_each(constructor);
	}
}

cosmos::cosmos(const std::size_t reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
	significant.meta.settings.si.set_pixels_per_meter(100.f);
	entity_debug_names[0] = "dead entity";

	set_standard_behaviour_trees(*this);
}

cosmos::cosmos(const cosmos& b) {
	*this = b;
}

const std::string& cosmos::get_debug_name(const entity_id id) const {
	return entity_debug_names[id.indirection_index + 1];
}

void cosmos::set_debug_name(const entity_id id, const std::string& new_debug_name) {
	entity_debug_names[id.indirection_index + 1] = new_debug_name;
}

void cosmos::delete_debug_name(const entity_id id) {
	entity_debug_names[id.indirection_index + 1] = "dead entity";
}

bool cosmos::operator==(const cosmos& b) const {
	return significant == b.significant;
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}

cosmos& cosmos::operator=(const cosmos_significant_state& b) {
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
	
	//complete_reinference();
	profiler.complete_reinference.new_measurement();
	systems_inferred = b.systems_inferred;
	//reserve_storage_for_entities(get_maximum_entities());
	profiler.complete_reinference.end_measurement();
	return *this;
}

#if COSMOS_TRACKS_GUIDS

void cosmos::assign_next_guid(const entity_handle new_entity) {
	auto this_guid = significant.meta.next_entity_guid.value++;

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
	complete_reinference();
}

void cosmos::complete_reinference(const const_entity_handle h) {
	destroy_inferred_state_of(h);
	create_inferred_state_for(h);
}

void cosmos::reserve_storage_for_entities(const size_t n) {
	get_aggregate_pool().initialize_space(n);
	reserve_storage_for_all_components(n);
	entity_debug_names.resize(n + 1);

	auto reservation_lambda = [n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	};

	systems_inferred.for_each(reservation_lambda);
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", entities_count());
}

size_t cosmos::get_rng_seed_for(const entity_id id) const {
	size_t transform_hash = 0;
	const auto tr = get_handle(id).get_logic_transform();
	transform_hash = static_cast<size_t>(std::abs(tr.pos.x)*100.0);
	transform_hash += static_cast<size_t>(std::abs(tr.pos.y)*100.0);
	transform_hash += static_cast<size_t>(std::abs(tr.rotation)*100.0);

	return get_handle(id).get_guid() + transform_hash;
}

double cosmos::get_total_time_passed_in_seconds(const double view_interpolation_ratio) const {
	const auto double_dt = static_cast<double>(significant.meta.delta.in_seconds());

	return get_total_time_passed_in_seconds() + double_dt * view_interpolation_ratio;
}

double cosmos::get_total_time_passed_in_seconds() const {
	return significant.meta.total_steps_passed * static_cast<double>(significant.meta.delta.in_seconds());
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
	auto pooled_object_raw_id = get_aggregate_pool().allocate();

	return entity_handle(*this, pooled_object_raw_id);
}

entity_handle cosmos::create_entity(const std::string& debug_name) {
	auto new_entity = allocate_new_entity();
	set_debug_name(new_entity, debug_name);
	new_entity += components::guid();

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif
	//ensure(new_entity.get_id().indirection_index != 37046);
	//ensure(new_entity.get_id().indirection_index != 36985);
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

entity_handle cosmos::clone_entity(const entity_id source_entity_id) {
	entity_handle source_entity = get_handle(source_entity_id);

	if (source_entity.dead()) {
		return get_handle(entity_id());
	}

	ensure(
		!source_entity.has<components::child>() 
		&& "Cloning of entities with child component is not yet supported"
	);

	auto new_entity = allocate_new_entity();
	
	clone_all_components_except<
		/*
			These components will be cloned shortly,
			with due care to each of them.
		*/
		components::all_inferred_state,
		components::fixtures,
		components::child
	>(new_entity, source_entity);

	set_debug_name(new_entity, "+" + source_entity.get_debug_name());

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif

	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}

	new_entity.make_cloned_child_entities_recursive(source_entity_id);
	
	if (source_entity.has<components::fixtures>()) {
		/*
			Copy all properties from the component of the source entity except the owner_body field.
			Exactly as if we were creating that component by hand.
		*/

		components::fixtures fixtures = source_entity.get<components::fixtures>().get_raw_component();
		const auto owner_of_the_source = fixtures.owner_body;
		fixtures.owner_body = entity_id();

		new_entity += fixtures;

		/*
			Only now assign the owner_body in a controllable manner.
		*/

		const bool source_owns_itself = owner_of_the_source == source_entity;

		if (source_owns_itself) {
			/*
				If the fixtures of the source entity were owned by the same entity,
				let the cloned entity also own itself
			*/
			new_entity.set_owner_body(new_entity);
		}
		else {
			/*
				If the fixtures of the source entity were owned by a different entity,
				let the cloned entity also be owned by that different entity
			*/
			new_entity.set_owner_body(owner_of_the_source);
		}
	}

	if (source_entity.has<components::all_inferred_state>()) {
		new_entity.add(components::all_inferred_state());
	}

	return new_entity;
}

void cosmos::delete_entity(const entity_id e) {
	const auto handle = get_handle(e);
	
	ensure(handle.alive());

	const auto inferred = handle.find<components::all_inferred_state>();

	const bool should_deactivate_inferred_state_to_avoid_repeated_reinference = inferred != nullptr;

	if (should_deactivate_inferred_state_to_avoid_repeated_reinference) {
		inferred.set_activated(false);
	}

#if COSMOS_TRACKS_GUIDS
	clear_guid(handle);
#endif
	// now manipulation of an entity without all_inferred_state component won't trigger redundant reinference

	const auto maybe_fixtures = handle.find<components::fixtures>();

	if (maybe_fixtures != nullptr) {
		const auto owner_body = maybe_fixtures.get_owner_body();

		const bool should_release_dependency = owner_body != handle;

		if (should_release_dependency) {
			maybe_fixtures.set_owner_body(handle);
		}
	}

	free_all_components(get_handle(e));
	get_aggregate_pool().free(e);
	delete_debug_name(e);

	/*
		Unregister that id as a parent from the relational system
	*/

	systems_inferred.get<relational_system>().handle_deletion_of_potential_parent(e);
}

void cosmos::advance_deterministic_schemata(const logic_step_input input) {
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
	
	perform_transfers(step.input.entropy.transfer_requests, step);
	
	performance.entropy_length.measure(step.input.entropy.length());

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

	force_joint_system().apply_forces_towards_target_entities(step);
	item_system().handle_throw_item_intents(step);
	hand_fuse_system().init_explosions(step);

	performance.start(meter_type::PHYSICS);
	listener.during_step = true;
	systems_inferred.get<physics_system>().step_and_set_new_transforms(step);
	listener.during_step = false;
	performance.stop(meter_type::PHYSICS);
	rotation_copying_system().update_rotations(step.cosm);
	position_copying_system().update_transforms(step);

	trace_system().lengthen_sprites_of_traces(step);

	crosshair_system().generate_crosshair_intents(step);
	crosshair_system().apply_crosshair_intents_to_base_offsets(step);
	crosshair_system().apply_base_offsets_to_crosshair_transforms(step);

//	item_system().translate_gui_intents_to_transfer_requests(step);
	item_system().start_picking_up_items(step);
	item_system().pick_up_touching_items(step);

	damage_system().destroy_outdated_bullets(step);
	damage_system().destroy_colliding_bullets_and_send_damage(step);

	destruction_system().generate_damages_from_forceful_collisions(step);
	destruction_system().apply_damages_and_split_fixtures(step);
	
	sentience_system().regenerate_values_and_advance_spell_logic(step);
	sentience_system().apply_damage_and_generate_health_events(step);
	systems_inferred.get<physics_system>().post_and_clear_accumulated_collision_messages(step);
	sentience_system().cooldown_aimpunches(step);
	sentience_system().set_borders(step);

	driver_system().release_drivers_due_to_requests(step);
	driver_system().assign_drivers_who_touch_wheels(step);
	driver_system().release_drivers_due_to_ending_contact_with_wheel(step);

	particles_existence_system().game_responses_to_particle_effects(step);

	sound_existence_system().create_sounds_from_game_events(step);
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

	auto& transfers = step.transient.messages.get_queue<item_slot_transfer_request>();
	perform_transfers(transfers, step);

	particles_existence_system().displace_streams_and_destroy_dead_streams(step);
	sound_existence_system().destroy_dead_sounds(step);

	trace_system().destroy_outdated_traces(step);

	const size_t queued_before_marking_num = step.transient.messages.get_queue<messages::queue_destruction>().size();

	destroy_system().mark_queued_entities_and_their_children_for_deletion(step);

	trace_system().spawn_finishing_traces_for_deleted_entities(step);

	listener.~contact_listener();

	movement_system().generate_movement_events(step);

	animation_system().game_responses_to_animation_messages(step);

	animation_system().handle_animation_messages(step);
	animation_system().progress_animation_states(step);

	//position_copying_system().update_transforms(step);
	//rotation_copying_system().update_rotations(step.cosm);

	profiler.raycasts.measure(systems_inferred.get<physics_system>().ray_casts_since_last_step);

	++significant.meta.total_steps_passed;

	const size_t queued_at_end_num = step.transient.messages.get_queue<messages::queue_destruction>().size();

	ensure_eq(queued_at_end_num, queued_before_marking_num);

	performance.stop(meter_type::LOGIC);
}

void cosmos::perform_deletions(const logic_step step) {
	destroy_system().perform_deletions(step);
}
