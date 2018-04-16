#pragma once
#include <memory>
#include <type_traits>

#include "augs/enums/callback_result.h"

namespace augs {
	struct trivial_type_marker;
};

template <class M>
struct comparator {
	bool& equal;
	const M& first;

	template <class... Args>
	auto operator()(Args&&...) const {
		return callback_result::CONTINUE;
	}

	auto operator()(const M& resolved) const {
		if(!(first == resolved)) {
			equal = false;
			return callback_result::ABORT;
		}

		return callback_result::CONTINUE;
	}

	auto operator()(const augs::trivial_type_marker& resolved) const {
		if(!(first == *reinterpret_cast<const M*>(std::addressof(resolved)))) {
			equal = false;
			return callback_result::ABORT;
		}

		return callback_result::CONTINUE;
	}
};

template <class M, class id_type, class Container>
bool compare_all_fields_to(
	const M& first,
	const id_type& property_id,
	const entity_type_id type_id,
	const cosmos& cosm,
	const Container& ids
) {
	bool equal = true;

	property_id.access(
		cosm, 
		type_id, 
		ids, 
		comparator<M> { equal, first }
	);

	return equal;
}
