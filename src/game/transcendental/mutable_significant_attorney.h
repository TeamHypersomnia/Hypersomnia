#pragma once
#include "game/transcendental/entity_handle_declaration.h"

namespace augs {
	template <template <class T> class make_pool_id, class... components>
	class component_aggregate; 
}

struct cosmic_delta;
class cosmos;

class mutable_significant_attorney {
	/*
		The following domains are free to arbitrarily change the cosmos_solvable::significant,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/

	friend cosmos; 
	friend cosmic_delta;

	template <bool is_const>
	friend class basic_entity_handle;

	template <template <class T> class make_pool_id, class... components>
	friend class augs::component_aggregate; 

	mutable_significant_attorney() {}
};
