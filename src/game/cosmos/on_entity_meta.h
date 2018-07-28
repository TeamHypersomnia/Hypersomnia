#pragma once
#include "game/cosmos/cosmos_solvable.h"

template <class C, class F>
decltype(auto) cosmos_solvable::on_entity_meta_impl(
	C& self,
	const entity_id id,
	F callback	
) {
	using meta_ptr = maybe_const_ptr_t<std::is_const_v<C>, entity_solvable_meta>;

	if (id.type_id.is_set()) {
		return self.significant.entity_pools.visit(
			id.type_id,
			[&](auto& pool) -> decltype(auto) {	
				return callback(static_cast<meta_ptr>(pool.find(id.raw)));
			}
		);
	}
	else {
		return callback(static_cast<meta_ptr>(nullptr));
	}
}

template <class F>
decltype(auto) cosmos_solvable::on_entity_meta(const entity_id id, F&& callback) {
	return on_entity_meta_impl(*this, id, std::forward<F>(callback));
}	

template <class F>
decltype(auto) cosmos_solvable::on_entity_meta(const entity_id id, F&& callback) const {
	return on_entity_meta_impl(*this, id, std::forward<F>(callback));
}	

template <class F>
decltype(auto) cosmos_solvable::on_entity_meta(const entity_guid id, F&& callback) {
	return on_entity_meta_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
}	

template <class F>
decltype(auto) cosmos_solvable::on_entity_meta(const entity_guid id, F&& callback) const {
	return on_entity_meta_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
}	
