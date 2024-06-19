#pragma once
#include "augs/misc/pool/pool_io.hpp"
#include "game/cosmos/per_entity_type.h"
#include "augs/templates/for_each_type.h"
#include "augs/readwrite/memory_stream.h"

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		if constexpr(can_reserve_v<Archive>) {
			augs::byte_counter_stream counter_stream;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				augs::write_bytes(counter_stream, cosm.get_common_significant());
				augs::write_bytes(counter_stream, cosm.get_solvable().significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			
			into.reserve(into.get_write_pos() + counter_stream.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			augs::write_bytes(into, cosm.get_common_significant());
			augs::write_bytes(into, cosm.get_solvable().significant);
		}
	}

	/* 
		The cosmos will only ever be read/written in the context of an intercosm. 
		It will be refreshed (reinferred) by the i/o logic when it is the time.
	*/

	template <class Archive>
	void read_signi_bytes(Archive& from, cosmos& cosm) {
		cosmic::change_solvable_significant(cosm, [&](cosmos_solvable_significant& significant) {
			augs::read_bytes(from, significant);
			return changer_callback_result::DONT_REFRESH;
		});
	}

	template <class Archive>
	void read_common_bytes(Archive& from, cosmos& cosm) {
		cosm.change_common_significant([&](cosmos_common_significant& common) {
			augs::read_bytes(from, common);
			return changer_callback_result::DONT_REFRESH;
		});
	}

	template <class Archive>
	void read_object_bytes(Archive& from, cosmos& cosm) {
		auto& profiler = cosm.profiler;

		auto scope = measure_scope(profiler.deserialization_pass);

		read_common_bytes(from, cosm);
		read_signi_bytes(from, cosm);
	}

	template <class Archive>
	void write_object_bytes(Archive& ar, const intercosm& ic) {
		augs::write_bytes_no_overload(ar, ic);
	}

	template <class Archive>
	void read_object_bytes(Archive& ar, intercosm& ic) {
		ic.clear();
		augs::read_bytes_no_overload(ar, ic);
		ic.post_load_state_correction();
	}
}

template void augs::write_object_bytes(augs::memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::memory_stream&, cosmos&);

template void augs::write_object_bytes(augs::ref_memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::cref_memory_stream&, cosmos&);

template void augs::write_object_bytes(std::ofstream&, const cosmos&);
template void augs::read_object_bytes(std::ifstream&, cosmos&);


template void augs::write_object_bytes(augs::memory_stream&, const intercosm&);
template void augs::read_object_bytes(augs::memory_stream&, intercosm&);

template void augs::write_object_bytes(augs::ref_memory_stream&, const intercosm&);
template void augs::read_object_bytes(augs::cref_memory_stream&, intercosm&);

template void augs::write_object_bytes(std::ofstream&, const intercosm&);
template void augs::read_object_bytes(std::ifstream&, intercosm&);
