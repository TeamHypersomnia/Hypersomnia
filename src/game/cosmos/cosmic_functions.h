#pragma once
#include "game/cosmos/entity_flavour_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/cosmos/specific_entity_handle_declaration.h"
#include "game/common_state/entity_name_str.h"

class cosmic_delta;
class cosmos;

/*
	The purpose of this class is to centralize all functions 
	that can arbitrarily alter the solvable state inside the cosmos,
   	e.g. change fields of synchronized components and then refresh them accordingly.
*/

enum class reinference_type {
	NONE,
	ONLY_AFFECTED,
	ENTIRE_COSMOS
};

template <class E>
struct entity_solvable;

class cosmic {
	static void destroy_caches_of(const entity_handle& h);
	static void infer_all_entities(cosmos& cosm);

	template <class F>
	friend void entity_deleter(const entity_handle, F);

	template <class E, class C, class I, class P>
	static ref_typed_entity_handle<E> specific_create_entity_detail(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		const I& initial_components,
		P pre_construction
	);

public:
	static void set_specific_name(const entity_handle&, const entity_name_str&);
	static void clear(cosmos& cosm);

	template <class C, class I, class E>
	static ref_typed_entity_handle<E> specific_paste_entity(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		const I& initial_components
	);

	template <class C, class E, class P>
	static ref_typed_entity_handle<E> specific_create_entity(
		C& cosm, 
		const typed_entity_flavour_id<E> flavour_id,
		P&& pre_construction
	);

	static entity_handle create_entity(
		cosmos& cosm,
		entity_flavour_id
	);

	template <class C, class... Types, class Pre, class Post>
	static entity_handle create_entity(
		C& cosm,
		const constrained_entity_flavour_id<Types...> flavour_id,
		Pre&& pre_construction,
		Post post_construction
	);

	template <class C, class I, class E>
	static ref_typed_entity_handle<E> undo_delete_entity(
		C& cosm,
		const I undo_delete_input,
		const entity_solvable<E>& deleted_content,
		const reinference_type reinference
	);

	template <class E>
	static void make_suitable_for_cloning(entity_solvable<E>& solvable);

	template <class handle_type, class P>
	static auto specific_clone_entity(
		const handle_type source_entity,
		P&& pre_construction
	);

	template <class handle_type>
	static auto specific_clone_entity(const handle_type source_entity);

	static void undo_last_create_entity(const entity_handle);
	static std::optional<cosmic_pool_undo_free_input> delete_entity(const entity_handle);

	static void reserve_storage_for_entities(cosmos&, const cosmic_pool_size_type s);
	static void increment_step(cosmos&);

	static void reinfer_solvable(cosmos&);
	static void reinfer_all_entities(cosmos&);
	static void infer_caches_for(const entity_handle& h);

	template <class C, class F>
	static void change_solvable_significant(C& cosm, F&& callback);

	template <template <class> class Predicate = always_true, class C, class F>
	static void for_each_entity(C& self, F callback);

	static void after_solvable_copy(cosmos&, const cosmos&);
	static void set_flavour_id_cache_enabled(bool flag, cosmos&);
};
