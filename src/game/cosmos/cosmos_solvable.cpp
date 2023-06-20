#include "game/cosmos/cosmos_solvable.h"

#include "augs/templates/introspect.h"
#include "game/cosmos/typed_entity_handle_declaration.h"
#include "game/cosmos/cosmos_solvable.hpp"
#include "game/cosmos/on_entity_meta.h"
#include "augs/misc/pool/pool_allocate.h"

const cosmos_solvable cosmos_solvable::zero;

void cosmos_solvable::clear() {
	destroy_all_caches();
	significant.entity_pools.clear();
	significant.clk = {};
	significant.specific_names.clear();
}

std::size_t cosmos_solvable::get_entities_count() const {
	return significant.entity_pools.size();
}

bool cosmos_solvable::empty() const {
	return get_entities_count() == 0;
}

cosmos_solvable::cosmos_solvable(const cosmic_pool_size_type reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
}

static auto make_reserver(const std::size_t n) {
	return [n](auto, auto& sys) {
		using T = remove_cref<decltype(sys)>;

		if constexpr(can_reserve_caches_v<T>) {
			sys.reserve_caches_for_entities(n);
		}
	};
}

void cosmos_solvable::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	significant.entity_pools.reserve(n);
	augs::introspect(make_reserver(n), inferred);
}

void cosmos_solvable::destroy_all_caches() {
	inferred.~cosmos_solvable_inferred();

	significant.entity_pools.for_each_container(
		[&](auto& entity_pool) {
			using E = entity_type_of<typename remove_cref<decltype(entity_pool)>::value_type>;

			for_each_type_in_list<typename E::synchronized_arrays>(
				[&](auto t) {
					using T = decltype(t);

					if constexpr(T::is_cache) {
						auto& caches = entity_pool.template get_corresponding_array<T>();

						const auto n = caches.size();
						caches.clear();
						caches.resize(n);
					}
				}
			);
		}
	);

	new (&inferred) cosmos_solvable_inferred;
}

void cosmos_solvable::increment_step() {
	++significant.clk.now.step;
}

double cosmos_solvable::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos_solvable::get_total_seconds_passed() const {
	return significant.clk.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos_solvable::get_total_steps_passed() const {
	return significant.clk.now.step;
}

augs::stepped_timestamp cosmos_solvable::get_timestamp() const {
	return significant.clk.now;
}

const augs::stepped_clock& cosmos_solvable::get_clock() const {
	return significant.clk;
}

augs::delta cosmos_solvable::get_fixed_delta() const {
	return significant.clk.dt;
}

void cosmos_solvable::set_steps_per_second(const unsigned steps) {
	significant.clk.dt = augs::delta::steps_per_second(steps);
}

unsigned cosmos_solvable::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

std::optional<cosmic_pool_undo_free_input> cosmos_solvable::free_entity(const entity_id id) {
	return significant.on_pool(id.type_id, [id](auto& p){ return p.free(id.raw); });
}

void cosmos_solvable::undo_last_allocate_entity(const entity_id id) {
	return significant.on_pool(id.type_id, [id](auto& p){ return p.undo_last_allocate(id.raw); });
}
