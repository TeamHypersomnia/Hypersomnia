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

#include <fstream>
#include <sstream>

void multiverse::save_cosmos_to_file(std::string filename) {
	writing_savefile.new_measurement();

	std::ofstream out(filename, std::ios::out | std::ios::binary);

	{
		cereal::PortableBinaryOutputArchive ar(out);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}

	writing_savefile.end_measurement();
}

void multiverse::load_cosmos_from_file(std::string filename) {
	ensure(main_cosmos == cosmos());

	reading_savefile.new_measurement();

	std::ifstream in(filename, std::ios::in | std::ios::binary);

	{
		cereal::PortableBinaryInputArchive ar(in);

		ar(main_cosmos_timer);
		ar(main_cosmos_manager);
		ar(main_cosmos);
	}

	reading_savefile.end_measurement();
}

bool cosmos::operator==(const cosmos& second) const {
	std::ostringstream this_serialized;
	std::ostringstream second_serialized;
	
	cosmos c1; c1.clone_significant_from(*this);
	cosmos c2; c2.clone_significant_from(second);

	{
		cereal::BinaryOutputArchive ar(this_serialized);
		ar(c1);
	}

	{
		cereal::BinaryOutputArchive ar(second_serialized);
		ar(c2);
	}

	const auto& this_str = this_serialized.rdbuf()->str();
	const auto& second_str = second_serialized.rdbuf()->str();
	
	bool cosmoi_identical = std::equal(this_str.begin(), this_str.end(), second_str.begin(), second_str.end()); 
	return cosmoi_identical;
}

bool cosmos::operator!=(const cosmos& second) const {
	return !operator==(second);
}