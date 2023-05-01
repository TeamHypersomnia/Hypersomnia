#pragma once
#include "game/cosmos/typed_entity_handle_declaration.h"

template <class T, class H>
auto& get_corresponding(const H& handle) {
	using entity_type = entity_type_of<H>;
	return handle.get_cosmos().get_solvable({}).significant.template get_pool<entity_type>().template get_corresponding<T>(handle.get_subject());
}
