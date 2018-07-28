#pragma once
#include "game/cosmos/cosmic_functions.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmos_solvable.hpp"

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
