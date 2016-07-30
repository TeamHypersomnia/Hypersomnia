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

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>

#include "entity_relations.h"
#include "multiverse.h"
#include "cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/filesystem/file.h"

#include <fstream>
#include <sstream>

void multiverse::save_cosmos_to_file(std::string filename) {
	writing_savefile.new_measurement();

	augs::output_stream_reserver reserver;

	{
		cereal::BinaryOutputArchiveReserver ar(reserver);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}

	auto stream = reserver.make_stream();

	{
		cereal::PortableBinaryOutputArchive ar(stream);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}

	writing_savefile.end_measurement();

	std::ofstream out(filename, std::ios::out | std::ios::binary);
	out.write(stream.buf.data(), stream.buf.size());
}

void multiverse::load_cosmos_from_file(std::string filename) {
	ensure(main_cosmos == cosmos());

	augs::input_stream stream;
	augs::assign_file_contents(filename, stream.buf);
	
	reading_savefile.new_measurement();

	{
		cereal::PortableBinaryInputArchive ar(stream);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}

	reading_savefile.end_measurement();
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
		cereal::BinaryOutputArchive ar(this_serialized);
		ar(c1);
	}

	{
		cereal::BinaryOutputArchive ar(second_serialized);
		ar(c2);
	}

	bool cosmoi_identical = this_serialized == second_serialized;
	return cosmoi_identical;
}

bool cosmos::significant_state::operator!=(const significant_state& second) const {
	return !operator==(second);
}