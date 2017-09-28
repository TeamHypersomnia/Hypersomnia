#include "augs/filesystem/file.h"

#include "augs/misc/templated_readwrite.h"
#include "augs/misc/streams.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"

#include "generated/introspectors.h"

#if ENTITY_TRACKS_NAME_FOR_DEBUG
/*
	Provide an i/o overload only for this compilation unit,
	where we need to write debug_name pointers as nullptrs,
	so that the operator== does not return false negatives.
*/

namespace augs {
	template <class... T>
	void read_object(augs::stream& ar, augs::component_aggregate<T...>& aggr) {
		ensure(false);
	}

	template <class... T>
	void write_object(augs::stream& ar, augs::component_aggregate<T...> aggr) {
		aggr.debug_name = nullptr;
		write_bytes(ar, &aggr, 1);
	}
}

static_assert(augs::has_io_overloads_v<augs::stream, put_all_components_into_t<augs::component_aggregate>>);
#endif

void cosmos_significant_state::clear() {
	*this = cosmos_significant_state();
}

std::size_t cosmos_significant_state::get_first_mismatch_pos(const cosmos_significant_state& second) const {
	/*
		Notice that the comparison is performed by byte-wise serialization.
		This means that it won't work if entity_ids are internally defined as pointers. The must be some ids assigned in a deterministic manner.
		Thus, it makes sense to compare two cosmoi, if we apply the same operations upon both, as a determinism test.

		It should not be, however, used to compare two cosmoi for whom there were different patterns of allocations,
		so for example if one cosmos A is initially empty, and three entities are created and deleted for a test,
		and then two exactly same entities are created both in this cosmos and in some other freshly created cosmos B,
		A will not be equal to B, firstly because guids will differ, and secondly because pool-generated entity_ids in the components may have differing values,
		despite pointing to the same entities logically.
	*/

	augs::output_stream_reserver this_serialized_reserver;
	augs::output_stream_reserver second_serialized_reserver;

	auto& r1 = this_serialized_reserver;
	auto& r2 = second_serialized_reserver;

	augs::write(r1, *this);
	augs::write(r2, second);
	
	auto this_serialized = r1.make_stream();
	auto second_serialized = r2.make_stream();

	augs::write(this_serialized, *this);
	augs::write(second_serialized, second);

	//size_t mismatch_byte = 0;
	//bool found_mismatch = false;
	//
	//for (size_t i = 0; i < this_serialized.size(); ++i) {
	//	if (this_serialized[i] != second_serialized[i]) {
	//		mismatch_byte = i;
	//		found_mismatch = true;
	//		LOG("%x %x", int(this_serialized[i]), int(second_serialized[i]));
	//		break;
	//	}
	//}
	
	//cosmos_significant_state resultant;
	//augs::read(second_serialized, resultant);

	//if(found_mismatch)
	//	LOG("C1: %x\nC2: %x, mismatch: %x", this_serialized.to_string(), second_serialized.to_string(), mismatch_byte);

	if (this_serialized.size() == second_serialized.size()) {
		const auto mismatch = this_serialized.mismatch(second_serialized);

		if (mismatch == this_serialized.size()) {
			return -1;
		}
		
		return mismatch;
	}

	return std::min(this_serialized.size(), second_serialized.size());
}

bool cosmos_significant_state::operator==(const cosmos_significant_state& second) const {
	return get_first_mismatch_pos(second) == -1;
}

bool cosmos_significant_state::operator!=(const cosmos_significant_state& second) const {
	return !operator==(second);
}