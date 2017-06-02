#include "augs/misc/templated_readwrite.h"
#include "augs/misc/streams.h"

#include "cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/filesystem/file.h"
#include "generated/introspectors.h"

void cosmos::save_to_file(const std::string& filename) {
	profiler.total_save.new_measurement();
	
	auto& reserved_memory = reserved_memory_for_serialization;

	// memory reservation stage
	{
		profiler.size_calculation_pass.new_measurement();
		augs::output_stream_reserver reserver;

		augs::write(reserver, significant);
		
		profiler.size_calculation_pass.end_measurement();
		profiler.memory_allocation_pass.new_measurement();

		reserved_memory.reserve(static_cast<size_t>(reserver.size() * 1.2));

		profiler.memory_allocation_pass.end_measurement();
	}

	// writing stage

	profiler.serialization_pass.new_measurement();

	augs::write(reserved_memory, significant);

	profiler.serialization_pass.end_measurement();
	profiler.writing_savefile.new_measurement();

	augs::create_binary_file(filename, reserved_memory);

	profiler.writing_savefile.end_measurement();
	profiler.total_save.end_measurement();
}

bool cosmos::load_from_file(const std::string& filename) {
	if (augs::file_exists(filename)) {
		profiler.total_load.new_measurement();

		ensure(significant.pool_for_aggregates.empty());

		//ensure(cosmos() == cosmos());
		//ensure(main_cosmos == cosmos());

		auto& stream = reserved_memory_for_serialization;

		profiler.reading_savefile.new_measurement();

		augs::get_file_contents_binary_into(filename, stream);

		profiler.reading_savefile.end_measurement();
		profiler.deserialization_pass.new_measurement();

		augs::read(stream, significant);

		profiler.deserialization_pass.end_measurement();

		refresh_for_new_significant_state();

		profiler.total_load.end_measurement();

		return true;
	}

	return false;
}

std::size_t cosmos_significant_state::get_first_mismatch_pos(const cosmos_significant_state& second) const {
	/*
		Notice that the comparison is performed by byte-wise serialization.
		This means that even if entity_id's are internally defined as pointers, they will be written as some ids in a deterministic manner.
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