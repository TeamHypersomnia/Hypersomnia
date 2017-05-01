#include <tuple>
#include "game/transcendental/cosmos.h"

#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_in_types.h"
#include "cosmic_delta.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "augs/misc/templated_readwrite.h"

#include "augs/misc/streams.h"
#include "augs/misc/delta_compression.h"

#include "augs/misc/pooled_object_id.h"

#include "generated_introspectors.h"

/* Several assumptions regarding delta encoding */

static_assert(
	sizeof(entity_id) >= sizeof(entity_guid)
	&& alignof(entity_id) >= alignof(entity_guid),
	"With given memory layouts, entity_id<->entity_guid substitution will not be possible in delta encoding"
);

static_assert(!has_introspect_v<cosmos>, "Trait has failed");
static_assert(!has_introspect_v<augs::trivial_variant<int, double>>, "Trait has failed");
static_assert(has_introspect_v<cosmos_metadata>, "Trait has failed");
static_assert(has_introspect_v<augs::constant_size_vector<int, 2>>, "Trait has failed");
static_assert(has_introspect_v<zeroed_pod<unsigned int>>, "Trait has failed");

template <class T>
void transform_component_ids_to_guids_in_place(
	T& comp,
	const cosmos& cosm
) {
	augs::introspect_recursive<
		bind_types_t<std::is_base_of, entity_id>,
		always_recurse,
		stop_recursion_if_valid
	>(
		[&cosm](auto, auto& id) {
			const auto handle = cosm[id];

			id.unset();
			
			auto& guid_inside_ref = reinterpret_cast<entity_guid&>(id);

			if (handle.alive()) {
				guid_inside_ref = handle.get_guid();
			}
			else {
				guid_inside_ref = entity_guid();
			}
		},
		comp
	);
}

template <class T>
void transform_component_guids_to_ids_in_place(
	T& comp,
	const cosmos& cosm
) {
	augs::introspect_recursive<
		bind_types_t<std::is_base_of, entity_id>,
		always_recurse,
		stop_recursion_if_valid
	> (
		[&cosm](auto, auto& id) {
			const auto guid_inside = reinterpret_cast<const entity_guid&>(id);

			id.unset();

			if (guid_inside != 0) {
				id = cosm.guid_map_for_transport.at(guid_inside);
			}
		},
		comp
	);
}

struct delted_stream_of_entities {
	unsigned new_entities = 0;
	unsigned changed_entities = 0;
	unsigned deleted_entities = 0;

	augs::stream stream_of_new_guids;
	augs::stream stream_for_new;
	augs::stream stream_for_changed;
	augs::stream stream_for_deleted;
};

bool cosmic_delta::encode(
	const cosmos& base, 
	const cosmos& enco, 
	augs::stream& out
) {
	const auto used_bits = out.size();
	//should_eq(0, used_bits);

	enco.profiler.delta_encoding.new_measurement();
	typedef decltype(base.significant.pool_for_aggregates)::element_type aggregate;
	
	delted_stream_of_entities dt;

	enco.significant.pool_for_aggregates.for_each_object_and_id([&](const aggregate& agg, const entity_id id) {
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
		std::fill(overridden_components.begin(), overridden_components.end(), false);

		augs::stream new_content;
		
		augs::introspect(
			[&agg, &base, &enco, &entity_changed, &new_content, &overridden_components](
				auto label,
				const auto& base_id, 
				const auto& enco_id
			) {
				typedef std::decay_t<decltype(enco_id)> encoded_id_type;
				typedef typename encoded_id_type::element_type component_type;

				if (std::is_same<component_type, components::guid>::value) {
					return;
				}

				constexpr size_t idx = index_in_list_v<encoded_id_type, decltype(agg.component_ids)>;

				const auto base_c = base.get_component_pool<component_type>()[base_id];
				const auto enco_c = enco.get_component_pool<component_type>()[enco_id];

				if (enco_c.dead() && base_c.dead()) {
					return;
				}
				else if (enco_c.dead() && base_c.alive()) {
					ensure(false && "Error! A previously existing component does not exist now.");
					std::terminate();
					return;
				}
				else if (enco_c.alive() && base_c.dead()) {
					component_type base_compo;
					component_type enco_compo = enco_c.get();

					transform_component_ids_to_guids_in_place(enco_compo, enco);

					augs::write_delta(base_compo, enco_compo, new_content, true);

					entity_changed = true;
					overridden_components[idx] = true;
				}
				else {
					component_type base_compo = base_c.get();
					component_type enco_compo = enco_c.get();

					transform_component_ids_to_guids_in_place(base_compo, base);
					transform_component_ids_to_guids_in_place(enco_compo, enco);

					if (augs::write_delta(base_compo, enco_compo, new_content)) {
						entity_changed = true;
						overridden_components[idx] = true;
					}
				}
			},
			base_components,
			enco_components
		);

		if (is_new) {
#if COSMOS_TRACKS_GUIDS
			augs::write(dt.stream_of_new_guids, stream_written_id);
#else
			// otherwise new entity_id assignment needs be deterministic
#endif

			augs::write_flags(dt.stream_for_new, overridden_components);
			augs::write(dt.stream_for_new, new_content);

			++dt.new_entities;
		}
		else if (entity_changed) {
			augs::write(dt.stream_for_changed, stream_written_id);

			augs::write_flags(dt.stream_for_changed, overridden_components);
			augs::write(dt.stream_for_changed, new_content);

			++dt.changed_entities;
		}
	});

	base.significant.pool_for_aggregates.for_each_object_and_id([&base, &enco, &out, &dt](const aggregate&, const entity_id id) {
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
			++dt.deleted_entities;
			augs::write(dt.stream_for_deleted, stream_written_id);
		}
	});

	augs::stream new_meta_content;

	const bool has_meta_changed = augs::write_delta(
		base.significant.meta, 
		enco.significant.meta, 
		new_meta_content, 
		true
	);

	const bool has_anything_changed = 
		has_meta_changed
		|| dt.new_entities
		|| dt.changed_entities 
		|| dt.deleted_entities
	;

	if (has_anything_changed) {
		augs::write(out, true);

		augs::write(out, new_meta_content);

		augs::write(out, dt.new_entities);
		augs::write(out, dt.changed_entities);
		augs::write(out, dt.deleted_entities);

		augs::write(out, dt.stream_of_new_guids);
		augs::write(out, dt.stream_for_new);
		augs::write(out, dt.stream_for_changed);
		augs::write(out, dt.stream_for_deleted);
	}
	else {
		augs::write(out, false);
	}

	enco.profiler.delta_encoding.end_measurement();

	enco.profiler.delta_bytes.measure(out.size());
	base.profiler.delta_bytes.measure(out.size());

	return has_anything_changed;
}

void cosmic_delta::decode(
	cosmos& deco, 
	augs::stream& in, 
	const bool resubstantiate_partially
) {
	if (in.get_unread_bytes() == 0) {
		return;
	}

	bool has_anything_changed = false;

	augs::read(in, has_anything_changed);

	if (in.failed()) {
		return;
	}

	if (!has_anything_changed) {
		return;
	}
	
	deco.profiler.delta_decoding.new_measurement();

	deco.destroy_inferred_state_completely();

	augs::read_delta(deco.significant.meta, in, true);

	delted_stream_of_entities dt;

	augs::read(in, dt.new_entities);
	augs::read(in, dt.changed_entities);
	augs::read(in, dt.deleted_entities);

	size_t new_guids = dt.new_entities;
	std::vector<entity_id> new_entities_ids;

	while (dt.new_entities--) {
#if COSMOS_TRACKS_GUIDS
		entity_guid new_guid;

		augs::read(in, new_guid);
		
		if (in.failed()) {
			return;
		}

		new_entities_ids.emplace_back(deco.create_entity_with_specific_guid("delta_created", new_guid));
#else
		// otherwise new entity_id assignment needs be deterministic
#endif
	}

	for(const auto new_entity_id : new_entities_ids) {
		const auto new_entity = deco[new_entity_id];

		std::array<bool, COMPONENTS_COUNT> overridden_components;

		augs::read_flags(in, overridden_components);

		const auto& agg = new_entity.get();
		const auto& deco_components = agg.component_ids;

		for_each_through_std_get(
			deco_components,
			[&agg, &overridden_components, &new_entity, &in, &deco](const auto& deco_id) {
				typedef std::decay_t<decltype(deco_id)> encoded_id_type;
				typedef typename encoded_id_type::element_type component_type;

				if (std::is_same<component_type, components::guid>::value) {
					return;
				}

				constexpr size_t idx = index_in_list_v<encoded_id_type, decltype(agg.component_ids)>;
				
				if (overridden_components[idx]) {
					component_type decoded_component;

					augs::read_delta(decoded_component, in, true);
					transform_component_guids_to_ids_in_place(decoded_component, deco);

					new_entity.allocator::add(decoded_component);
				}
			}
		);
	}

	while (dt.changed_entities--) {
		entity_guid guid_of_changed;
		
		augs::read(in, guid_of_changed);
		
		if (in.failed()) {
			return;
		}

		const auto changed_entity = deco.get_handle(guid_of_changed);

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		augs::read_flags(in, overridden_components);

		const auto& agg = changed_entity.get();
		const auto& deco_components = agg.component_ids;

		for_each_through_std_get(
			deco_components,
			[&agg, &overridden_components, &in, &deco, &changed_entity](const auto& deco_id) {
				typedef std::decay_t<decltype(deco_id)> encoded_id_type;
				typedef typename encoded_id_type::element_type component_type;

				if (std::is_same<component_type, components::guid>::value) {
					return;
				}
				
				constexpr size_t idx = index_in_list_v<encoded_id_type, decltype(agg.component_ids)>;

				if (overridden_components[idx]) {
					const auto deco_c = deco.get_component_pool<component_type>()[deco_id];

					if (deco_c.dead()) {
						component_type decoded_component;

						augs::read_delta(decoded_component, in, true);
						
						transform_component_guids_to_ids_in_place(decoded_component, deco);
						
						changed_entity.allocator::add(decoded_component);
					}
					else {
						component_type decoded_component = deco_c.get();

						transform_component_ids_to_guids_in_place(decoded_component, deco);
						augs::read_delta(decoded_component, in);
						transform_component_guids_to_ids_in_place(decoded_component, deco);

						changed_entity.allocator::get<component_type>() = decoded_component;
					}
				}
			}
		);
	}

	while (dt.deleted_entities--) {
#if COSMOS_TRACKS_GUIDS
		entity_guid guid_of_destroyed;

		augs::read(in, guid_of_destroyed);
		
		if(in.failed()) {
			return;
		}

		deco.delete_entity(deco.get_handle(guid_of_destroyed));
#else
		static_assert(false, "Unimplemented");
#endif
	}

	const auto unread_bits = in.get_unread_bytes();
	//should_eq(0, unread_bits);

	deco.create_inferred_state_completely();

	deco.profiler.delta_decoding.end_measurement();
}

#include "augs/build_settings/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include <gtest/gtest.h>

TEST(CosmicDelta, PaddingSanityCheck1) {
	struct ok {
		bool a;
		int b;
		bool c;

		ok() : a(false), b(1), c(false) {

		}
	};

	typedef ok checked_type;
	constexpr size_t type_size = sizeof(checked_type);

	char buf1[type_size];
	char buf2[type_size];

	for (int i = 0; i < type_size; ++i) {
		buf1[i] = 3;
		buf2[i] = 4;
	}

	new (buf1) checked_type;
	new (buf2) checked_type;

	const bool are_different = memcmp(buf1, buf2, type_size);

	ASSERT_TRUE(are_different);
}

TEST(CosmicDelta, PaddingSanityCheck2) {
	struct ok {
		bool a = false;
		int b = 1;
		bool c = false;
	};

	typedef ok checked_type;
	constexpr size_t type_size = sizeof(checked_type);

	char buf1[type_size];
	char buf2[type_size];

	for (int i = 0; i < type_size; ++i) {
		buf1[i] = 3;
		buf2[i] = 4;
	}

	new (buf1) checked_type;
	new (buf2) checked_type;

	const bool are_different = memcmp(buf1, buf2, type_size);

	ASSERT_TRUE(are_different);
}

TEST(CosmicDelta, CosmicDeltaPaddingTest) {
	auto padding_checker = [](auto c, auto... args) {
		typedef decltype(c) component_type;
		static_assert(std::is_same_v<std::decay_t<component_type>, component_type>, "Something's wrong with the types");
		static_assert(augs::is_byte_io_safe_v<augs::stream, component_type>, "Non-trivially copyable component detected!");
		
		typedef component_type checked_type;
		constexpr size_t type_size = sizeof(checked_type);

		char buf1[type_size];
		char buf2[type_size];

		for (int i = 0; i < type_size; ++i) {
			buf1[i] = 3;
			buf2[i] = 4;
		}

		new (buf1) checked_type(args...);
		new (buf2) checked_type(args...);

		int iter = 0;
		bool same = true;

		for (; iter < type_size; ++iter) {
			if (buf1[iter] != buf2[iter]) {
				same = false;
				break;
			}
		}

		ASSERT_TRUE(same) << "Padding is wrong in " << typeid(checked_type).name() << "\nsizeof: " << type_size << "\nDivergence position: " << iter;
	};

	padding_checker(augs::window::event::change());
	//padding_checker(game_gui_element_location());
	
	struct dum {
	//	game_gui_rect_world rect_world;
		int dragged_charges = 0;

		bool is_gui_look_enabled = false;
		bool preview_due_to_item_picking_request = false;
		bool draw_space_available_inside_container_icons = true;
		padding_byte pad;

		//hotbar_button hotbar_buttons[9];
		//drag_and_drop_target_drop_item drop_item_icon = augs::gui::material();

		dum() 
			//:  
			//drop_item_icon(augs::gui::material(assets::game_image_id::DROP_HAND_ICON, red))
		{
		}
	};

	LOG_NVPS(sizeof(dum));
	sizeof(dum);
	LOG_NVPS(sizeof(entity_handle));

	padding_checker(dum());
	//padding_checker(std::array<hotbar_button, 9>());
	//padding_checker(drag_and_drop_target_drop_item(augs::gui::material()), augs::gui::material());

	for_each_through_std_get(put_all_components_into_t<std::tuple>(), padding_checker);
}

TEST(Cosmos, GuidizeTests) {
	cosmos c1(2);

	const auto new_ent1 = c1.create_entity("e1");

	item_slot_transfer_request dt;
	dt.item = new_ent1;

	const auto guidized = c1.guidize(dt);

	EXPECT_EQ(0, guidized.target_slot.container_entity);
	EXPECT_EQ(1, guidized.item);

	const auto deguidized = c1.deguidize(guidized);
	EXPECT_EQ(dt.item, deguidized.item);
	EXPECT_EQ(dt.target_slot.container_entity, deguidized.target_slot.container_entity);
	EXPECT_EQ(entity_id(), deguidized.target_slot.container_entity);
	// sanity check
	EXPECT_EQ(entity_id(), dt.target_slot.container_entity);
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
	new_ent1 += components::rigid_body();
	new_ent1 += components::render();
	new_ent1 += components::sprite();
	
	new_ent2 += components::transform();
	new_ent2 += components::trace();
	new_ent2 += components::position_copying();

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	const auto ent1 = c1.get_handle(first_guid);
	const auto ent2 = c1.get_handle(second_guid);

	// check if components are intact after encode/decode cycle


	ASSERT_EQ(2, c1.entities_count());
	ASSERT_EQ(2, c2.entities_count());
	ASSERT_TRUE(ent1.has<components::transform>());
	const bool transform_intact = ent1.get<components::transform>() == first_transform;
	ASSERT_TRUE(transform_intact);
	ASSERT_TRUE(ent1.has<components::rigid_body>());
	ASSERT_TRUE(ent1.has<components::render>());
	ASSERT_TRUE(ent1.has<components::sprite>());
	ASSERT_FALSE(ent1.has<components::trace>());

	ASSERT_TRUE(ent2.has<components::transform>());
	const bool default_transform_intact = ent2.get<components::transform>() == components::transform();
	ASSERT_TRUE(default_transform_intact);
	ASSERT_FALSE(ent2.has<components::rigid_body>());
	ASSERT_FALSE(ent2.has<components::render>());
	ASSERT_FALSE(ent2.has<components::sprite>());
	ASSERT_TRUE(ent2.has<components::trace>());

	{
		augs::stream comparatory;
		
		ASSERT_FALSE(cosmic_delta::encode(c1, c2, comparatory));

		ASSERT_EQ(1, comparatory.size());
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

	new_ent1 += components::position_copying();
	new_ent2 += components::position_copying();
	new_ent3 += components::position_copying();

	new_ent1.get<components::position_copying>().set_target(new_ent2);
	new_ent2.get<components::position_copying>().set_target(new_ent3);
	new_ent3.get<components::position_copying>().set_target(new_ent1);

	new_ent1 += components::sentience();
	new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	ASSERT_EQ(3, c1.entities_count());
	ASSERT_EQ(3, c2.entities_count());

	const auto ent1 = c1.get_handle(first_guid);
	const auto ent2 = c1.get_handle(second_guid);
	const auto ent3 = c1.get_handle(third_guid);

	ASSERT_TRUE(ent1.has<components::sentience>());
	ASSERT_TRUE(ent1.has<components::position_copying>());
	const bool pc1_intact = ent1.get<components::position_copying>().target == ent2.get_id();
	const bool pc1ch_intact = ent1[child_entity_name::CHARACTER_CROSSHAIR] == ent2.get_id();
	ASSERT_TRUE(pc1_intact);

	ASSERT_TRUE(ent2.has<components::position_copying>());
	const bool pc2_intact = ent2.get<components::position_copying>().target == ent3.get_id();
	ASSERT_TRUE(pc2_intact);

	ASSERT_TRUE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	ASSERT_TRUE(pc3_intact);

	{
		augs::stream comparatory;

		ASSERT_FALSE(cosmic_delta::encode(c1, c2, comparatory));

		ASSERT_EQ(1, comparatory.size());
	}
}


TEST(CosmicDelta, CosmicDeltaThreeEntitiesWithReferencesAndDestroyedChild) {
	entity_guid c1_first_guid = 0;
	entity_guid c1_second_guid = 0;
	entity_guid c1_third_guid = 0;
	entity_guid c2_first_guid = 0;
	entity_guid c2_second_guid = 0;
	entity_guid c2_third_guid = 0;

	cosmos c1(3);
	{
		const auto new_ent1 = c1.create_entity("e1");
		const auto new_ent2 = c1.create_entity("e2");
		const auto new_ent3 = c1.create_entity("e3");

		c1_first_guid = new_ent1.get_guid();
		c1_second_guid = new_ent2.get_guid();
		c1_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying();
		new_ent2 += components::position_copying();
		new_ent3 += components::position_copying();

		new_ent1.get<components::position_copying>().set_target(new_ent2);
		new_ent2.get<components::position_copying>().set_target(new_ent3);
		new_ent3.get<components::position_copying>().set_target(new_ent1);

		new_ent1 += components::sentience();
		new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	cosmos c2(3);
	{
		const auto new_ent1 = c2.create_entity("e1");
		const auto new_ent2 = c2.create_entity("e2");
		const auto new_ent3 = c2.create_entity("e3");

		c2_first_guid = new_ent1.get_guid();
		c2_second_guid = new_ent2.get_guid();
		c2_third_guid = new_ent3.get_guid();

		new_ent1 += components::position_copying();
		new_ent2 += components::position_copying();
		new_ent3 += components::position_copying();

		new_ent1.get<components::position_copying>().set_target(new_ent2);
		new_ent2.get<components::position_copying>().set_target(new_ent3);
		new_ent3.get<components::position_copying>().set_target(new_ent1);

		new_ent1 += components::sentience();
		new_ent1.map_child_entity(child_entity_name::CHARACTER_CROSSHAIR, new_ent2);
	}

	ASSERT_EQ(3, c1.entities_count());
	ASSERT_EQ(3, c2.entities_count());

	{
		augs::stream comparatory;

		ASSERT_FALSE(cosmic_delta::encode(c1, c2, comparatory));

		ASSERT_EQ(1, comparatory.size());
	}

	c2.delete_entity(c2.get_handle(c2_second_guid));
	ASSERT_EQ(2, c2.entities_count());

	{
		augs::stream s;

		cosmic_delta::encode(c1, c2, s);
		cosmic_delta::decode(c1, s);
	}

	{
		augs::stream comparatory;

		ASSERT_FALSE(cosmic_delta::encode(c1, c2, comparatory));

		ASSERT_EQ(1, comparatory.size());
	}

	ASSERT_EQ(2, c1.entities_count());

	const auto ent1 = c1.get_handle(c1_first_guid);
	ASSERT_FALSE(c1.entity_exists_with_guid(c1_second_guid));
	const auto ent3 = c1.get_handle(c1_third_guid);

	ASSERT_TRUE(ent1.has<components::position_copying>());
	const bool pc1_dead = c1[ent1.get<components::position_copying>().target].dead();
	ASSERT_TRUE(pc1_dead);

	ASSERT_TRUE(ent3.has<components::position_copying>());
	const bool pc3_intact = ent3.get<components::position_copying>().target == ent1.get_id();
	ASSERT_TRUE(pc3_intact);
}

#endif