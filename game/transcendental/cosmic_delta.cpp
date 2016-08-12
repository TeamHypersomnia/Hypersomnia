#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/misc/streams.h"
#include "augs/misc/delta_compression.h"

#include "augs/misc/pool_id.h"

#include "BitStream.h"
#include "cosmos.h"
#include "cosmic_delta.h"

template <class T>
bool write_delta(const T& base, const T& enco, RakNet::BitStream& out, const bool write_changed_bit = false) {
	const auto dt = augs::delta_encode(base, enco);
	const bool has_changed = dt.changed_bytes.size() > 0;

	if (write_changed_bit)
		augs::write_object(out, has_changed);

	if (has_changed) {
		augs::write_vector_short(out, dt.changed_bytes);
		augs::write_vector_short(out, dt.changed_offsets);
	}

	return has_changed;
}

template <class T>
void read_delta(T& deco, RakNet::BitStream& in, const bool read_changed_bit = false) {
	augs::object_delta dt;

	bool has_changed = true;

	if (read_changed_bit)
		augs::read_object(in, has_changed);

	if (has_changed) {
		augs::read_vector_short(new_content, dt.changed_bytes);
		augs::read_vector_short(new_content, dt.changed_offsets);

		augs::delta_decode(deco, dt);
	}
}

struct per_entity_delta {
	unsigned new_entities = 0;
	unsigned changed_entities = 0;
	unsigned removed_entities = 0;

	RakNet::BitStream stream_for_new;
	RakNet::BitStream stream_for_changed;
	RakNet::BitStream stream_for_removed;
};

void cosmic_delta::encode(const cosmos& base, const cosmos& enco, RakNet::BitStream& out) {
	enco.profiler.delta_encoding.new_measurement();
	typedef decltype(base.significant.pool_for_aggregates)::element_type aggregate;

	per_entity_delta dt;

	enco.significant.pool_for_aggregates.for_each_with_id([&base, &enco, &dt](const aggregate& agg, entity_id id) {
		const const_entity_handle enco_entity = enco.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto stream_written_id = enco_entity.get_guid();
		const auto maybe_base_entity = base.guid_map_for_transport.find(stream_written_id);

		const bool is_new = maybe_base_entity == base.guid_map_for_transport.end();
		const entity_id base_entity_id = is_new ? entity_id() : (*maybe_base_entity).second;

		const const_entity_handle base_entity = base[base_entity_id];
#else
		const_entity_handle base_entity = base.get_handle(id);
		bool is_new = base_entity.dead();
		auto stream_written_id = id;
#endif

		const auto& base_components = is_new ? aggregate::component_id_tuple() : base_entity.get().component_ids;
		const auto& enco_components = agg.component_ids;

		bool entity_changed = false;

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		std::array<bool, COMPONENTS_COUNT> removed_components;

		RakNet::BitStream new_content;
		
		for_each_in_tuples(base_components, enco_components,
			[&overridden_components, &removed_components, &entity_changed, &agg, &enco, &base, &new_content](const auto& enco_id, const auto& base_id) {
			typedef std::decay_t<decltype(enco_id)> encoded_id_type;

			constexpr size_t idx = index_in_tuple<encoded_id_type, decltype(agg.component_ids)>::value;
			const auto enco_c = enco[enco_id];
			const auto base_c = base[base_id];

			if (enco_c.dead() && base_c.alive()) {
				removed_components[idx] = true;
				entity_changed = true;
				return;
			}

			typedef typename encoded_id_type::element_type component_type;
			
			component_type base_compo = base_c.alive() ? base_c.get() : component_type();
			component_type enco_compo = enco_c.alive() ? enco_c.get() : component_type();

			held_id_introspector<component_type>::for_each_held_id(base_compo, [&base](entity_id& id) {
				unsigned guid = base[id].get_guid();
				id = entity_id();
				//id.guid = guid;
			});

			held_id_introspector<component_type>::for_each_held_id(enco_compo, [&enco](entity_id& id) {
				unsigned guid = enco[id].get_guid();
				id = entity_id();
				//id.guid = guid;
			});

			if (write_delta(base_compo, enco_compo, new_content)) {
				entity_changed = true;
				overridden_components[idx] = true;
			}
		}
		);
		
		if (is_new) {
#if COSMOS_TRACKS_GUIDS
			augs::write_object(new_content, stream_written_id);
#else
			// otherwise new entity_id assignment needs be deterministic
#endif

			for (const bool flag : overridden_components)
				augs::write_object(new_content, flag);

			augs::write_object(dt.stream_for_new, new_content);

			++dt.new_entities;
		}
		else if (entity_changed) {
			augs::write_object(new_content, stream_written_id);

			for (const bool flag : overridden_components)
				augs::write_object(new_content, flag);

			for (const bool flag : removed_components)
				augs::write_object(new_content, flag);

			augs::write_object(dt.stream_for_changed, new_content);
			
			++dt.changed_entities;
		}
	});

	base.significant.pool_for_aggregates.for_each_with_id([&base, &enco, &out, &dt](const aggregate&, entity_id id) {
		const const_entity_handle base_entity = base.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto maybe_enco_entity = enco.guid_map_for_transport.find(base_entity.get_guid());
		const bool is_dead = maybe_enco_entity == enco.guid_map_for_transport.end();
#else
		const const_entity_handle enco_entity = enco.get_handle(id);
		const bool is_dead = enco_entity.dead();
#endif

		if (is_dead) {
			++dt.removed_entities;
			augs::write_object(dt.stream_for_removed, id);
		}
	});

	RakNet::BitStream new_meta_content;

	const bool meta_changed = write_delta(base.significant.meta, enco.significant.meta, new_meta_content, true);

	const bool has_anything_changed = meta_changed || dt.new_entities || dt.changed_entities || dt.removed_entities;

	if (has_anything_changed) {
		augs::write_object(out, new_meta_content);

		augs::write_object(out, dt.new_entities);
		augs::write_object(out, dt.changed_entities);
		augs::write_object(out, dt.removed_entities);

		augs::write_object(out, dt.stream_for_new);
		augs::write_object(out, dt.stream_for_changed);
		augs::write_object(out, dt.stream_for_removed);
	}

	enco.profiler.delta_encoding.end_measurement();
}

void cosmic_delta::decode(cosmos& into, RakNet::BitStream& in, bool resubstantiate_partially) {
	if (in.GetNumberOfUnreadBits() == 0)
		return;
	
	into.profiler.delta_decoding.new_measurement();


	into.profiler.delta_decoding.end_measurement();
}