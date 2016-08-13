#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/misc/streams.h"
#include "augs/misc/delta_compression.h"

#include "augs/misc/pool_id.h"

#include "BitStream.h"
#include "cosmos.h"
#include "cosmic_delta.h"

#include <gtest/gtest.h>

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
		augs::read_vector_short(in, dt.changed_bytes);
		augs::read_vector_short(in, dt.changed_offsets);

		augs::delta_decode(deco, dt);
	}
}

struct delted_entity_stream {
	unsigned new_entities = 0;
	unsigned changed_entities = 0;
	unsigned removed_entities = 0;

	RakNet::BitStream stream_of_new_guids;
	RakNet::BitStream stream_for_new;
	RakNet::BitStream stream_for_changed;
	RakNet::BitStream stream_for_removed;
};

void cosmic_delta::encode(const cosmos& base, const cosmos& enco, RakNet::BitStream& out) {
	const auto used_bits = out.GetNumberOfBitsUsed();
	ensure_eq(0, used_bits);

	enco.profiler.delta_encoding.new_measurement();
	typedef decltype(base.significant.pool_for_aggregates)::element_type aggregate;

	delted_entity_stream dt;

	enco.significant.pool_for_aggregates.for_each_with_id([&base, &enco, &dt](const aggregate& agg, const entity_id id) {
		const const_entity_handle enco_entity = enco.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto stream_written_id = enco_entity.get_guid();
		const auto maybe_base_entity = base.guid_map_for_transport.find(stream_written_id);

		const bool is_new = maybe_base_entity == base.guid_map_for_transport.end();
		const entity_id base_entity_id = is_new ? entity_id() : (*maybe_base_entity).second;

		const const_entity_handle base_entity = base[base_entity_id];
#else
		const const_entity_handle base_entity = base.get_handle(id);
		const bool is_new = base_entity.dead();
		const auto stream_written_id = id;
#endif

		const auto& base_components = is_new ? aggregate::component_id_tuple() : base_entity.get().component_ids;
		const auto& enco_components = agg.component_ids;

		bool entity_changed = false;

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		std::array<bool, COMPONENTS_COUNT> removed_components;
		std::fill(overridden_components.begin(), overridden_components.end(), false);
		std::fill(removed_components.begin(), removed_components.end(), false);

		RakNet::BitStream new_content;
		
		for_each_in_tuples(base_components, enco_components,
			[&overridden_components, &removed_components, &entity_changed, &agg, &enco, &base, &new_content](const auto& base_id, const auto& enco_id) {
			typedef std::decay_t<decltype(enco_id)> encoded_id_type;
			typedef typename encoded_id_type::element_type component_type;

			if (std::is_same<component_type, components::guid>::value)
				return;

			constexpr size_t idx = index_in_tuple<encoded_id_type, decltype(agg.component_ids)>::value;

			const auto base_c = base[base_id];
			const auto enco_c = enco[enco_id];

			if (enco_c.dead() && base_c.dead())
				return;
			else if (enco_c.dead() && base_c.alive()) {
				entity_changed = true;
				removed_components[idx] = true;
				return;
			}
			else if (enco_c.alive() && base_c.dead()) {
				component_type base_compo;
				component_type enco_compo = enco_c.get();

				transform_component_ids_to_guids(enco_compo, enco);

				write_delta(base_compo, enco_compo, new_content, true);

				entity_changed = true;
				overridden_components[idx] = true;
			}
			else {
				component_type base_compo = base_c.get();
				component_type enco_compo = enco_c.get();

				transform_component_ids_to_guids(base_compo, base);
				transform_component_ids_to_guids(enco_compo, enco);

				if (write_delta(base_compo, enco_compo, new_content)) {
					entity_changed = true;
					overridden_components[idx] = true;
				}
			}
		}
		);
		
		if (is_new) {
#if COSMOS_TRACKS_GUIDS
			augs::write_object(dt.stream_of_new_guids, stream_written_id);
#else
			// otherwise new entity_id assignment needs be deterministic
#endif

			for (const bool flag : overridden_components)
				augs::write_object(dt.stream_for_new, flag);

			augs::write_object(dt.stream_for_new, new_content);

			++dt.new_entities;
		}
		else if (entity_changed) {
			augs::write_object(dt.stream_for_changed, stream_written_id);

			for (const bool flag : overridden_components)
				augs::write_object(dt.stream_for_changed, flag);

			for (const bool flag : removed_components)
				augs::write_object(dt.stream_for_changed, flag);

			augs::write_object(dt.stream_for_changed, new_content);
			
			++dt.changed_entities;
		}
	});

	base.significant.pool_for_aggregates.for_each_with_id([&base, &enco, &out, &dt](const aggregate&, const entity_id id) {
		const const_entity_handle base_entity = base.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto stream_written_id = base_entity.get_guid();
		const auto maybe_enco_entity = enco.guid_map_for_transport.find(stream_written_id);
		const bool is_dead = maybe_enco_entity == enco.guid_map_for_transport.end();
#else
		const auto stream_written_id = id;
		const const_entity_handle enco_entity = enco.get_handle(stream_written_id);
		const bool is_dead = enco_entity.dead();
#endif

		if (is_dead) {
			++dt.removed_entities;
			augs::write_object(dt.stream_for_removed, stream_written_id);
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

		augs::write_object(out, dt.stream_of_new_guids);
		augs::write_object(out, dt.stream_for_new);
		augs::write_object(out, dt.stream_for_changed);
		augs::write_object(out, dt.stream_for_removed);

		out.AlignWriteToByteBoundary();
	}

	enco.profiler.delta_encoding.end_measurement();
}

void cosmic_delta::decode(cosmos& deco, RakNet::BitStream& in, const bool resubstantiate_partially) {
	if (in.GetNumberOfUnreadBits() == 0)
		return;
	
	deco.profiler.delta_decoding.new_measurement();

	read_delta(deco.significant.meta, in, true);

	delted_entity_stream dt;

	augs::read_object(in, dt.new_entities);
	augs::read_object(in, dt.changed_entities);
	augs::read_object(in, dt.removed_entities);

	size_t new_guids = dt.new_entities;
	std::vector<entity_handle> new_entities;

	while (dt.new_entities--) {
#if COSMOS_TRACKS_GUIDS
		unsigned new_guid;
		augs::read_object(in, new_guid);
		
		new_entities.emplace_back(deco.create_entity_with_specific_guid("delta_created", new_guid));
#else
		// otherwise new entity_id assignment needs be deterministic
#endif
	}

	for(const auto& new_entity : new_entities) {
		std::array<bool, COMPONENTS_COUNT> overridden_components;

		for (bool& flag : overridden_components)
			augs::read_object(in, flag);

		const auto& agg = new_entity.get();
		const auto& deco_components = agg.component_ids;

		for_each_in_tuple(deco_components,
			[&overridden_components, &new_entity, &agg, &deco, &in](const auto& deco_id) {
			typedef std::decay_t<decltype(deco_id)> encoded_id_type;
			typedef typename encoded_id_type::element_type component_type;

			if (std::is_same<component_type, components::guid>::value)
				return;

			constexpr size_t idx = index_in_tuple<encoded_id_type, decltype(agg.component_ids)>::value;
			
			if (overridden_components[idx]) {
				component_type decoded_component;

				read_delta(decoded_component, in, true);
				transform_component_guids_to_ids(decoded_component, deco);

				new_entity.allocator::add(decoded_component);
			}
		}
		);
	}

	while (dt.changed_entities--) {
		unsigned guid_of_changed;
		augs::read_object(in, guid_of_changed);

		const auto changed_entity = deco.get_entity_by_guid(guid_of_changed);

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		std::array<bool, COMPONENTS_COUNT> removed_components;

		for (bool& flag : overridden_components)
			augs::read_object(in, flag);

		for (bool& flag : removed_components)
			augs::read_object(in, flag);

		const auto& agg = changed_entity.get();
		const auto& deco_components = agg.component_ids;

		for_each_in_tuple(deco_components,
			[&overridden_components, &removed_components, &changed_entity, &agg, &deco, &in](const auto& deco_id) {
			typedef std::decay_t<decltype(deco_id)> encoded_id_type;
			typedef typename encoded_id_type::element_type component_type;

			if (std::is_same<component_type, components::guid>::value)
				return;

			constexpr size_t idx = index_in_tuple<encoded_id_type, decltype(agg.component_ids)>::value;

			if (overridden_components[idx]) {
				const auto deco_c = deco[deco_id];

				if (deco_c.dead()) {
					component_type decoded_component;

					read_delta(decoded_component, in, true);
					
					transform_component_guids_to_ids(decoded_component, deco);
					
					changed_entity.allocator::add(decoded_component);
				}
				else {
					component_type decoded_component = deco_c.get();

					transform_component_ids_to_guids(decoded_component, deco);
					read_delta(decoded_component, in);
					transform_component_guids_to_ids(decoded_component, deco);

					changed_entity.allocator::get<component_type>() = decoded_component;
				}
			}
			else if (removed_components[idx]) {
				changed_entity.remove<component_type>();
			}
		}
		);
	}

	while (dt.removed_entities--) {
#if COSMOS_TRACKS_GUIDS
		unsigned guid_of_destroyed;
		augs::read_object(in, guid_of_destroyed);

		deco.delete_entity(deco.get_entity_by_guid(guid_of_destroyed));
#else
		static_assert(false, "Unimplemented");
#endif
	}

	in.AlignReadToByteBoundary();
	
	const auto unread_bits = in.GetNumberOfUnreadBits();
	ensure_eq(0, unread_bits);

	deco.profiler.delta_decoding.end_measurement();
}

TEST(CosmicDelta, CosmicDeltaEmptyAndTwoNew) {
	cosmos c1(2);
	cosmos c2(2);

	const auto new_ent1 = c2.create_entity("e1");
	const auto new_ent2 = c2.create_entity("e2");

	const auto first_guid = new_ent1.get_guid();
	const auto second_guid = new_ent2.get_guid();

	components::transform first_transform(21, 0, 12.4f);

	new_ent1 += first_transform;
	new_ent1 += components::physics();
	new_ent1 += components::render();
	new_ent1 += components::sprite();
	
	new_ent2 += components::transform();
	new_ent2 += components::trace();
	new_ent2 += components::position_copying();

	{
		RakNet::BitStream s;

		cosmic_delta::encode(c1, c2, s);

		s.ResetReadPointer();

		cosmic_delta::decode(c1, s);
	}

	const auto ent1 = c1.get_entity_by_guid(first_guid);
	const auto ent2 = c1.get_entity_by_guid(second_guid);

	// check if components are intact after encode/decode cycle


	ASSERT_EQ(2, c1.entities_count());
	ASSERT_EQ(2, c2.entities_count());
	ASSERT_TRUE(ent1.has<components::transform>());
	const bool transform_intact = ent1.get<components::transform>() == first_transform;
	ASSERT_TRUE(transform_intact);
	ASSERT_TRUE(ent1.has<components::physics>());
	ASSERT_TRUE(ent1.has<components::render>());
	ASSERT_TRUE(ent1.has<components::sprite>());
	ASSERT_FALSE(ent1.has<components::trace>());

	ASSERT_TRUE(ent2.has<components::transform>());
	const bool default_transform_intact = ent2.get<components::transform>() == components::transform();
	ASSERT_TRUE(default_transform_intact);
	ASSERT_FALSE(ent2.has<components::physics>());
	ASSERT_FALSE(ent2.has<components::render>());
	ASSERT_FALSE(ent2.has<components::sprite>());
	ASSERT_TRUE(ent2.has<components::trace>());

	{
		RakNet::BitStream comparatory;
		
		cosmic_delta::encode(c1, c2, comparatory);

		ASSERT_EQ(0, comparatory.GetNumberOfBitsUsed());
		ASSERT_TRUE(c1 == c2);
	}
}

TEST(CosmicDelta, CosmicDeltaEmptyAndCreatedThreeEntitiesWithReferences) {
	cosmos c1(3);
	cosmos c2(3);

	const auto new_ent1 = c2.create_entity("e1");
	const auto new_ent2 = c2.create_entity("e2");
	const auto new_ent3 = c2.create_entity("e3");

	const auto first_guid = new_ent1.get_guid();
	const auto second_guid = new_ent2.get_guid();
	const auto third_guid = new_ent3.get_guid();

	new_ent1 += components::position_copying(new_ent2);
	new_ent2 += components::position_copying(new_ent3);
	new_ent3 += components::position_copying(new_ent1);

	new_ent1.map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, new_ent2);

	{
		RakNet::BitStream s;

		cosmic_delta::encode(c1, c2, s);

		s.ResetReadPointer();

		cosmic_delta::decode(c1, s);
	}

	ASSERT_EQ(3, c1.entities_count());
	ASSERT_EQ(3, c2.entities_count());

	const auto ent1 = c1.get_entity_by_guid(first_guid);
	const auto ent2 = c1.get_entity_by_guid(second_guid);
	const auto ent3 = c1.get_entity_by_guid(third_guid);

	ASSERT_TRUE(ent1.has<components::position_copying>());
	const bool pc1_intact = ent1.get<components::position_copying>().target == ent2.get_id();
	ASSERT_TRUE(pc1_intact);

	ASSERT_TRUE(ent2.has<components::position_copying>());
	const bool pc2_intact = ent2.get<components::position_copying>().target == ent3.get_id();
	ASSERT_TRUE(pc2_intact);

	ASSERT_TRUE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	ASSERT_TRUE(pc3_intact);

	ASSERT_TRUE(ent1.has<components::sub_entities>());
	const bool sub_entities_intact = ent1.get<components::sub_entities>().sub_entities_by_name[sub_entity_name::CHARACTER_CROSSHAIR] == ent2.get_id();
	ASSERT_TRUE(sub_entities_intact);

	{
		RakNet::BitStream comparatory;

		cosmic_delta::encode(c1, c2, comparatory);

		ASSERT_EQ(0, comparatory.GetNumberOfBitsUsed());
	}
}


TEST(CosmicDelta, CosmicDeltaThreeEntitiesWithReferencesAndDestroyedChild) {
	unsigned c1_first_guid = 0;
	unsigned c1_second_guid = 0;
	unsigned c1_third_guid = 0;
	unsigned c2_first_guid = 0;
	unsigned c2_second_guid = 0;
	unsigned c2_third_guid = 0;

	cosmos c1(3);
	{
		const auto new_ent1 = c1.create_entity("e1");
		const auto new_ent2 = c1.create_entity("e2");
		const auto new_ent3 = c1.create_entity("e3");

		c1_first_guid = new_ent1.get_guid();
		c1_second_guid = new_ent2.get_guid();
		c1_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying(new_ent2);
		new_ent2 += components::position_copying(new_ent3);
		new_ent3 += components::position_copying(new_ent1);

		new_ent1.map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	cosmos c2(3);
	{
		const auto new_ent1 = c2.create_entity("e1");
		const auto new_ent2 = c2.create_entity("e2");
		const auto new_ent3 = c2.create_entity("e3");

		c2_first_guid = new_ent1.get_guid();
		c2_second_guid = new_ent2.get_guid();
		c2_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying(new_ent2);
		new_ent2 += components::position_copying(new_ent3);
		new_ent3 += components::position_copying(new_ent1);

		new_ent1.map_sub_entity(sub_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	ASSERT_EQ(3, c1.entities_count());
	ASSERT_EQ(3, c2.entities_count());

	{
		RakNet::BitStream comparatory;

		cosmic_delta::encode(c1, c2, comparatory);

		ASSERT_EQ(0, comparatory.GetNumberOfBitsUsed());
	}

	c2.delete_entity(c2.get_entity_by_guid(c2_second_guid));
	ASSERT_EQ(2, c2.entities_count());

	{
		RakNet::BitStream s;

		cosmic_delta::encode(c1, c2, s);

		s.ResetReadPointer();

		cosmic_delta::decode(c1, s);
	}

	{
		RakNet::BitStream comparatory;

		cosmic_delta::encode(c1, c2, comparatory);

		ASSERT_EQ(0, comparatory.GetNumberOfBitsUsed());
	}

	ASSERT_EQ(2, c1.entities_count());

	const auto ent1 = c1.get_entity_by_guid(c1_first_guid);
	ASSERT_FALSE(c1.entity_exists_with_guid(c1_second_guid));
	const auto ent3 = c1.get_entity_by_guid(c1_third_guid);

	ASSERT_TRUE(ent1.has<components::position_copying>());
	const bool pc1_dead = c1[ent1.get<components::position_copying>().target].dead();
	ASSERT_TRUE(pc1_dead);

	ASSERT_TRUE(ent1.has<components::sub_entities>());
	const bool sub_entity_dead = c1[ent1.get<components::sub_entities>().sub_entities_by_name[sub_entity_name::CHARACTER_CROSSHAIR]].dead();
	ASSERT_TRUE(sub_entity_dead);

	ASSERT_TRUE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	ASSERT_TRUE(pc3_intact);
}