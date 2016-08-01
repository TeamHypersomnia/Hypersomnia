#include "augs/misc/streams.h"

#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/bitset.hpp>

#include <cereal/archives/portable_binary_hacked.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary_hacked.hpp>

#include "entity_relations.h"
#include "multiverse.h"
#include "cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/filesystem/file.h"

#include <fstream>
#include <sstream>

void multiverse::save_cosmos_to_file(std::string filename) {
	total_save.new_measurement();

	size_calculation_pass.new_measurement();
	augs::output_stream_reserver reserver;

	{
		cereal::BinaryOutputArchiveReserver ar(reserver);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos.significant);
	}
	size_calculation_pass.end_measurement();

	auto& stream = main_cosmos.reserved_memory_for_serialization;

	memory_allocation_pass.new_measurement();
	stream.reserve(static_cast<size_t>(reserver.size() * 1.2));
	memory_allocation_pass.end_measurement();

	serialization_pass.new_measurement();
	{
		cereal::PortableBinaryOutputArchiveHacked ar(stream);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}
	serialization_pass.end_measurement();

	writing_savefile.new_measurement();
	write_file_binary(filename, stream.buf);
	writing_savefile.end_measurement();
	
	total_save.end_measurement();
}

void multiverse::load_cosmos_from_file(std::string filename) {
	total_load.new_measurement();
	ensure(main_cosmos == cosmos());

	auto& stream = main_cosmos.reserved_memory_for_serialization;
	
	reading_savefile.new_measurement();
	augs::assign_file_contents_binary(filename, stream.buf);
	reading_savefile.end_measurement();
	
	stream.reset_pos();

	deserialization_pass.new_measurement();

	{
		cereal::PortableBinaryInputArchiveHacked ar(stream);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}
	deserialization_pass.end_measurement();

	total_load.end_measurement();
}


bool cosmos::significant_state::operator==(const significant_state& second) const {
	augs::output_stream_reserver this_serialized_reserver;
	augs::output_stream_reserver second_serialized_reserver;
	
	significant_state c1 = *this;
	significant_state c2 = second;

	{
		cereal::BinaryOutputArchiveReserver ar(this_serialized_reserver);
		ar(c1);
	}

	{
		cereal::BinaryOutputArchiveReserver ar(second_serialized_reserver);
		ar(c2);
	}

	auto this_serialized = this_serialized_reserver.make_stream();
	auto second_serialized = second_serialized_reserver.make_stream();

	{
		cereal::BinaryOutputArchiveHacked ar(this_serialized);
		ar(c1);
	}

	{
		cereal::BinaryOutputArchiveHacked ar(second_serialized);
		ar(c2);
	}

	bool cosmoi_identical = this_serialized == second_serialized;
	return cosmoi_identical;
}

bool cosmos::significant_state::operator!=(const significant_state& second) const {
	return !operator==(second);
}