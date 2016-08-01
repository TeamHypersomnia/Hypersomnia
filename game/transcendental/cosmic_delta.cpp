#include "augs/misc/streams.h"

//#include <cereal/cereal.hpp>
//#include <cereal/types/array.hpp>
//#include <cereal/types/vector.hpp>
//#include <cereal/types/string.hpp>
//#include <cereal/types/map.hpp>
//#include <cereal/types/set.hpp>
//#include <cereal/types/unordered_map.hpp>
//#include <cereal/types/utility.hpp>
//#include <cereal/types/tuple.hpp>
//#include <cereal/types/bitset.hpp>
//
//#include <cereal/archives/binary_hacked.hpp>

#include "cosmos.h"
#include "entity_relations.h"

void cosmos::delta_encode(cosmos& base, augs::stream& out) const {
	//augs::output_stream_reserver base_reserver;
	//augs::output_stream_reserver helper_reserver;
	//
	//entity_relations rr1;
	//entity_relations rr2;
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//rr2.sub_entities.push_back(entity_id());
	//
	//cereal::DeltaEncoderHelper base_helper(base_reserver);
	//cereal::DeltaEncoderHelper encoded_helper(helper_reserver);
	//
	//base_helper(rr1);
	//encoded_helper(rr2);
	//
	//LOG("rr1 Bounds count: %x", base_helper.first_order_containers.size());
	//LOG("rr2 Bounds count: %x", encoded_helper.first_order_containers.size());
	//
	//LOG("rr1 Last bound: %x, %x", (*base_helper.first_order_containers.rbegin()).first, (*base_helper.first_order_containers.rbegin()).second);
	//LOG("rr2 Last bound: %x, %x", (*encoded_helper.first_order_containers.rbegin()).first, (*encoded_helper.first_order_containers.rbegin()).second);

	//out.reserve(out.size() + reserver.size);
}

void cosmos::delta_decode(augs::stream& in, bool resubstantiate_partially) {

}