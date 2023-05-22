#pragma once
#include "game/inferred_caches/inferred_cache_common.h"
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

#include "game/cosmos/entity_type_traits.h"
#include "game/cosmos/entity_handle_declaration.h"

class cosmos;

class organism_cache {
public:
	using organism_id_type = typed_entity_id<dynamic_decoration>;
private:

	struct grid {
		struct cell {
			std::vector<organism_id_type> organisms;
		};

		ltrb aabb;
		std::vector<cell> cells;

		void reset(ltrb aabb);
		void clear();

		cell& get_cell(vec2i);
		const cell& get_cell(vec2i) const;

		vec2i get_cell_coord_at_world(vec2) const;

		cell& get_cell_at_world(vec2);
		const cell& get_cell_at_world(vec2) const;

		vec2i cells_size() const;
		bool is_set() const;

		template <class F>
		void for_each_cell(const ltrb query, F callback) const;

		void erase_organism_from_position(organism_id_type, vec2);
		void erase_organism(organism_id_type);
	};

	inferred_cache_map<grid> grids;

	grid* find_grid(const unversioned_entity_id);

	template <class E>
	bool assign_to_grid(const E&);

	template <class E>
	bool reset_grid_for(const E& area);
	void erase_organism(organism_id_type);

public:
	using cell_type = grid::cell;
	using grid_type = grid;

	template <class E>
	struct is_potential_organism {
		static constexpr bool value = 
			has_all_of_v<E, invariants::movement_path>
		;
	};

	template <class E>
	static constexpr bool is_potential_organism_v = is_potential_organism<E>::value;

	template <class E>
	struct concerned_with {
		static constexpr bool value = 
			is_potential_organism_v<E> || has_all_of_v<E, invariants::area_marker>
		;
	};	

	void reserve_caches_for_entities(const size_t n);

	void infer_all(const cosmos&);

	template <class E>
	void specific_infer_cache_for(const E&);

	void infer_cache_for(const const_entity_handle&);
	void destroy_cache_of(const const_entity_handle&);

	template <class E>
	void recalculate_grid(const E& area);

	template <class F>
	void for_each_cell_of_grid(const unversioned_entity_id origin, const ltrb query, F&& callback) const;

	template <class F>
	void for_each_cell_of_all_grids(const ltrb query, F callback) const;

	bool recalculate_cell_for(const unversioned_entity_id origin, const organism_id_type organism_id, vec2 old_position, vec2 new_position);

	const grid* find_grid(const unversioned_entity_id) const;
};
