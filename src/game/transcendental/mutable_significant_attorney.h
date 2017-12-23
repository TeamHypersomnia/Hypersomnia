#pragma once
#include "game/transcendental/entity_handle_declaration.h"

template <bool, class D>
class relations_mixin;

namespace augs {
	template <template <class T> class make_pool_id, class... components>
	class component_aggregate; 
}

struct cosmic_delta;

class mutable_significant_attorney {
	/*
		The following domains are free to arbitrarily change the cosmos_solvable::significant,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/

	friend cosmic_delta;

	template <bool is_const>
	friend class basic_entity_handle;

	template <bool, class D>
	friend class relations_mixin;

	mutable_significant_attorney() {}
};
