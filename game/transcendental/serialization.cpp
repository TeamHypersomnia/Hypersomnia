#include "augs/misc/templated_readwrite.h"
#include "augs/misc/streams.h"

#include "entity_relations.h"
#include "multiverse.h"
#include "cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/filesystem/file.h"

void multiverse::save_cosmos_to_file(std::string filename) {
	total_save.new_measurement();
	
	size_calculation_pass.new_measurement();

	augs::output_stream_reserver reserver;
	
	ensure(main_cosmos.get_pool(augs::pool_id<components::particle_group>()).empty());
	ensure(main_cosmos.get_pool(augs::pool_id<components::pathfinding>()).empty());
	ensure(main_cosmos.get_pool(augs::pool_id<components::visibility>()).empty());

	augs::write_object(reserver, main_cosmos_timer);
	augs::write_object(reserver, main_cosmos_manager);
	augs::write_object(reserver, main_cosmos.significant);

	size_calculation_pass.end_measurement();
	
	auto& stream = main_cosmos.reserved_memory_for_serialization;
	
	memory_allocation_pass.new_measurement();

	stream.reserve(static_cast<size_t>(reserver.size() * 1.2));

	memory_allocation_pass.end_measurement();
	
	serialization_pass.new_measurement();

	augs::write_object(stream, main_cosmos_timer);
	augs::write_object(stream, main_cosmos_manager);
	augs::write_object(stream, main_cosmos.significant);

	serialization_pass.end_measurement();
	
	writing_savefile.new_measurement();

	write_file_binary(filename, stream.buf);

	writing_savefile.end_measurement();
	
	total_save.end_measurement();
}

void multiverse::load_cosmos_from_file(std::string filename) {
	total_load.new_measurement();

	ensure(main_cosmos.significant.pool_for_aggregates.empty());

	auto& stream = main_cosmos.reserved_memory_for_serialization;
	
	reading_savefile.new_measurement();

	augs::assign_file_contents_binary(filename, stream.buf);

	reading_savefile.end_measurement();
	
	stream.reset_pos();
	
	deserialization_pass.new_measurement();

	augs::read_object(stream, main_cosmos_timer);
	augs::read_object(stream, main_cosmos_manager);
	augs::read_object(stream, main_cosmos.significant);

	deserialization_pass.end_measurement();
	
	main_cosmos.complete_resubstantialization();

	total_load.end_measurement();
}


bool cosmos::significant_state::operator==(const significant_state& second) const {
	ensure(false);
	augs::output_stream_reserver this_serialized_reserver;
	augs::output_stream_reserver second_serialized_reserver;
	auto& r1 = this_serialized_reserver;
	auto& r2 = second_serialized_reserver;

	augs::write_object(r1, *this);
	augs::write_object(r2, second);
	
	auto this_serialized = r1.make_stream();
	auto second_serialized = r2.make_stream();

	augs::write_object(this_serialized, *this);
	augs::write_object(second_serialized, second);

	//size_t mismatch_byte = 0;
	//
	//for (size_t i = 0; i < this_serialized.size(); ++i) {
	//	if (this_serialized[i] != second_serialized[i]) {
	//		mismatch_byte = i;
	//		LOG("%x %x", int(this_serialized[i]), int(second_serialized[i]));
	//		break;
	//	}
	//}
	//
	//second_serialized.reset_pos();
	//
	//significant_state resultant;
	//augs::read_object(second_serialized, resultant);

	//LOG("C1: %x\nC2: %x, mismatch: %x", this_serialized.to_string(), second_serialized.to_string(), mismatch_byte);

	bool cosmoi_identical = this_serialized == second_serialized;
	return cosmoi_identical;
}

bool cosmos::significant_state::operator!=(const significant_state& second) const {
	ensure(false);
	return !operator==(second);
}