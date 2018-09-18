#pragma once
#include "game/cosmos/cosmic_functions.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmos_solvable.hpp"
#include "game/organization/for_each_entity_type.h"

template <class... MustHaveComponents, class C, class F>
void cosmic::for_each_entity(C& self, F callback) {
	self.get_solvable({}).template for_each_entity<MustHaveComponents...>(
		[&](auto& object, const auto iteration_index) -> decltype(auto) {
			using O = decltype(object);
			using E = entity_type_of<O>;
			using iterated_handle_type = basic_iterated_entity_handle<is_const_ref_v<O>, E>;
			
			return callback(iterated_handle_type(self, { object, iteration_index } ));
		}
	);
}

template <class... MustHaveComponents, class F>
void cosmos::for_each_having(F&& callback) {
	cosmic::for_each_entity<MustHaveComponents...>(*this, std::forward<F>(callback));
}

template <class... MustHaveComponents, class F>
void cosmos::for_each_having(F&& callback) const {
	cosmic::for_each_entity<MustHaveComponents...>(*this, std::forward<F>(callback));
}

template <class... MustHaveInvariants, class F>
void cosmos::for_each_flavour_having(F&& callback) const {
	for_each_entity_type([&](auto e) {
		using E = decltype(e);

		if constexpr(has_all_of_v<E, MustHaveInvariants...>) {
			for_each_id_and_object(
				get_flavours<E>(), 
				[&callback](const raw_entity_flavour_id& id, const entity_flavour<E>& flavour) {
					callback(typed_entity_flavour_id<E>(id), flavour);
				}
			);
		}
	});
}

