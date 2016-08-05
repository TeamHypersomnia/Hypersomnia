#include "augs/misc/streams.h"
#include "augs/misc/delta_compression.h"

#include "BitStream.h"
#include "cosmos.h"
#include "entity_relations.h"

#include "cosmic_delta.h"

struct entity_change_record {
	entity_id subject;

	std::bitset<COMPONENTS_COUNT> components_added;
	std::bitset<COMPONENTS_COUNT> components_changed;
	std::bitset<COMPONENTS_COUNT> components_removed;
	
	std::vector<augs::object_delta> component_delta_content;
};

struct new_entity_record {
	entity_id subject;

	std::bitset<COMPONENTS_COUNT> components_added;
	std::vector<augs::object_delta> component_delta_content;
};

class per_entity_delta {
	std::vector<entity_id> deleted;
	std::vector<entity_change_record> changed;
	std::vector<new_entity_record> created;
};

namespace augs {
	template<class A>
	void read_object(A& ar, per_entity_delta& dt) {
		read_object(ar, dt.deleted);
		read_object(ar, significant.delta);

		read_object(ar, significant.pools_for_components);
		read_object(ar, significant.pool_for_aggregates);
	}

	template<class A>
	void write_object(A& ar, const per_entity_delta& significant) {
		write_object(ar, significant.settings);
		write_object(ar, significant.delta);

		write_object(ar, significant.pools_for_components);
		write_object(ar, significant.pool_for_aggregates);
	}
}

void cosmic_delta::encode(const cosmos& base, const cosmos& encoded, RakNet::BitStream& out) {
	encoded.profiler.delta_encoding.new_measurement();

	typedef decltype(base.significant.pool_for_aggregates)::element_type aggregate;

	struct change_record {
		entity_id subject;
		RakNet::BitStream content;
	};

	std::vector<change_record> created, changed;
	std::vector<entity_id> deleted;

	encoded.significant.pool_for_aggregates.for_each_with_id([&base, &encoded, &out](const aggregate& agg, entity_id id) {
		const_entity_handle encoded_entity = encoded.get_handle(id);
		const_entity_handle base_entity = base.get_handle(id);

		if (base_entity.alive()) {
			std::array<bool, COMPONENTS_COUNT> overridden_components;

			const auto& base_components = base_entity.get().component_ids;
			const auto& encoded_components = agg.component_ids;

			for_each_in_tuples(agg.component_ids, base_entity.get().component_ids, [&encoded, &base](auto encoded_id, auto base_id) {
				if (encoded[encoded_id].alive() && base[base_id].alive()) {

				}
			});
		}
		else {

		}
	});

	base.significant.pool_for_aggregates.for_each_with_id([&base, &encoded, &out, &deleted](const aggregate& agg, entity_id id) {
		const_entity_handle encoded_entity = encoded.get_handle(id);
		const_entity_handle base_entity = base.get_handle(id);

		if (encoded_entity.dead() && base_entity.alive()) {
			deleted.push_back(encoded_entity);
		}
	});
	
	encoded.profiler.delta_encoding.end_measurement();
}

void cosmic_delta::decode(cosmos& into, RakNet::BitStream& in, bool resubstantiate_partially) {
	into.profiler.delta_decoding.new_measurement();

	into.profiler.delta_decoding.end_measurement();
}