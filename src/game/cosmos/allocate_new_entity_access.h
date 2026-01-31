#pragma once
#include "game/stateless_systems/item_system.h"
#include "game/stateless_systems/gun_system.h"
#include "game/stateless_systems/missile_system.h"
#include "game/stateless_systems/trace_system.h"
#include "game/stateless_systems/sentience_system.h"
#include "game/stateless_systems/movement_system.h"
#include "game/stateless_systems/sentience_system.h"

struct allocation_system;
class gun_system;
class sentience_system;
class movement_system;
struct build_arena_input;

namespace test_scenes {
	class testbed;
	class minimal_scene;
}

class game_gui_system;
struct cosmos_global_solvable;
struct perform_transfer_impl;

struct duplicate_entities_command;
struct instantiate_flavour_command;

class allocate_new_entity_access {
	/*
		We have to be EXTREMELY careful who do we allow to create entities.

		That is because if the entity pool expands,
		it will INVALIDATE all existing entity handles and pointers to components.
			Not just that, it might break the loop that iterates over all entities of a given type!

		This might not necessarily cause a SIGSEGV because most of the time,
		std::vector reallocates larger memory without moving it if it's possible.

		This is particularly insidiuos because we won't even know most of the time that we have a "heap use after free" crash lurking somewhere.
	*/

	/*
		This is the system that will perform queued allocations
		in a safe, handle-free environment.
	*/

	friend allocation_system;

	/*
		perform_transfer might need to clone a stackable item.
		set_specified_quanitity, however, is protected by this access class.
		The only way to perform_transfer with a non-whole quantity is by the protected set_specified_quantity.

		Thus, we can already give perform_transfer the relevant access.
	*/

	friend perform_transfer_impl;

	/* 
		Rationale: this is only used for testbed and we want to easily manipulate entity creations there.
		The handles there are mostly temporary.
	*/

	template <class C, class E, class F>
	friend auto create_test_scene_entity(C& cosm, const E enum_flavour, F&& callback);

	/*
		These will need to generate some equipment.
	*/

	friend test_scenes::minimal_scene;
	friend test_scenes::testbed;

	/* 
		Rationale: item system needs some fine control over reloading transfers.
		We'll be careful, I promise.

		We'll pre-reserve several entities in item pools to not invalidate any references.
	*/

	friend void item_system::advance_reloading_contexts(const logic_step step);

	/* 
		Rationale: gun system will allocate a lot of missiles and shells,
		and we want one of the most frequent case of allocations to be rather fast.

		This will only invalidate handles to shells and missiles which are pretty much never formed except in preconstruct/postconstruct callbacks.

		Gun system will also partially transfer cartridges,
		which can potentially spawn new entities.
			We'll pre-reserve in item pools per every iteration to not invalidate any references.
			We can then ensure that the pool is not at capacity.
	*/

	friend void gun_system::launch_shots_due_to_pressed_triggers(const logic_step step);

	/*
		Will generate dropped coins.
	*/

	friend void sentience_system::regenerate_values_and_advance_spell_logic(const logic_step step) const;

	/*
		Will spawn blood splatters when health damage is dealt.
	*/

	friend messages::health_event sentience_system::process_health_event(messages::health_event h, const logic_step step) const;

	/*
		Will spawn blood footsteps when stepping on blood decals.
	*/

	friend void movement_system::apply_movement_forces(const logic_step step);

	/* 
		Rationale: missile system will allocate a lot of remnants from missilesand shells,
		and we want one of the most frequent case of allocations to be rather fast.

		This will only invalidate handles to remnants which are pretty much never formed except in preconstruct/postconstruct callbacks.
	*/

	friend void missile_system::detonate_colliding_missiles(const logic_step);

	/* 
		Rationale: trace system will allocate a lot of remnants from missilesand shells,
		and we want one of the most frequent case of allocations to be rather fast.

		This will only invalidate handles to traces which are pretty much never formed except in preconstruct/postconstruct callbacks.
	*/

	friend void trace_system::spawn_finishing_traces_for_deleted_entities(const logic_step) const;

	/* 
		For allocating characters. No character handles are manipulated anywhere in proximity. 
	*/
	friend class test_mode;

	/* 
		I promise I'll be careful.
	*/

	friend class arena_mode;

	/*
		Mounting logic needs to perform some partial transfers.
	*/

	friend cosmos_global_solvable;

	/*
		Obviously this needs fine-access to returned ids.
		Also for generating equipment.

		We'll be careful, I promise.
	*/

	template <class A>
	friend void build_arena_from_editor_project(A arena_handle, const build_arena_input in);

	/*
		For queueing transfers from view.
		Since it's view, it's safe.
	*/

	friend game_gui_system;

	template <bool is_const>
	friend class basic_game_gui_context;

	/*
		Debugger setup.
	*/

	friend instantiate_flavour_command;
	friend duplicate_entities_command;

	allocate_new_entity_access() {}
};
