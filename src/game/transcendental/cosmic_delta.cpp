#include "cosmic_delta.h"

#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_in_types.h"

#include "augs/misc/templated_readwrite.h"
#include "augs/misc/delta_compression.h"
#include "augs/misc/pooled_object_id.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include "generated/introspectors.h"

/* Several assumptions regarding delta encoding */

template <class T>
void transform_component_ids_to_guids_in_place(
	T& comp,
	const cosmos& cosm
) {
	augs::introspect(augs::recursive([&](auto&& self, auto, auto& id) {
		using T = std::decay_t<decltype(id)>;

		if constexpr(std::is_base_of_v<entity_id, T>) {
			const auto handle = cosm[id];

			id.unset();

			auto& guid_inside_ref = reinterpret_cast<entity_guid&>(id);

			if (handle.alive()) {
				guid_inside_ref = handle.get_guid();
			}
			else {
				guid_inside_ref = entity_guid();
			}
		}
		else {
			if constexpr(is_variable_size_container_v<T>) {
				for (auto& e : id) {
					augs::introspect_if_not_leaf(augs::pass_self(self), e);
				}
			}
			else {
				augs::introspect_if_not_leaf(augs::pass_self(self), id);
			}
		}
	}), comp);
}

template <class T>
void transform_component_guids_to_ids_in_place(
	T& comp,
	const cosmos& cosm
) {
	augs::introspect(augs::recursive([&](auto&& self, auto, auto& id) {
		using T = std::decay_t<decltype(id)>;

		if constexpr(std::is_base_of_v<entity_id, T>) {
			const auto guid_inside = reinterpret_cast<const entity_guid&>(id);

			id.unset();

			if (guid_inside != 0) {
				id = cosm.guid_to_id.at(guid_inside);
			}
		}
		else {
			if constexpr(is_variable_size_container_v<T>) {
				for (auto& e : id) {
					augs::introspect_if_not_leaf(augs::pass_self(self), e);
				}
			}
			else {
				augs::introspect_if_not_leaf(augs::pass_self(self), id);
			}
		}
	}), comp);
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
	
	delted_stream_of_entities dt;

	enco.significant.pool_for_aggregates.for_each_object_and_id([&](const auto& enco_agg, const entity_id id) {
		const const_entity_handle enco_entity = enco.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto stream_written_id = enco_entity.get_guid();
		const auto maybe_base_entity = base.guid_to_id.find(stream_written_id);

		const bool is_new = maybe_base_entity == base.guid_to_id.end();
		const entity_id base_entity_id = is_new ? entity_id() : (*maybe_base_entity).second;

		const const_entity_handle base_entity = base[base_entity_id];
#else
		const const_entity_handle base_entity = base.get_handle(id);
		const bool is_new = base_entity.dead();
		const auto stream_written_id = id;
#endif

		bool has_entity_changed = false;

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		std::array<bool, COMPONENTS_COUNT> removed_components;
		std::fill(overridden_components.begin(), overridden_components.end(), false);
		std::fill(removed_components.begin(), removed_components.end(), false);

		augs::stream new_content;
		
		for_each_component_type(
			[&base, &enco, base_entity, is_new, enco_entity, &has_entity_changed, &new_content, &overridden_components, &removed_components](auto c) {
				using component_type = decltype(c);
		
				// guid components is handled separately by the cosmos

				if constexpr(!std::is_same_v<component_type, components::guid>) {
					constexpr size_t idx = component_index_v<component_type>;
		
					const auto maybe_base = is_new ? nullptr : base_entity.allocator::template find<component_type>();
					const auto maybe_enco = enco_entity.allocator::template find<component_type>();
		
					if (!maybe_enco && !maybe_base) {
						return;
					}
					else if (!maybe_enco && maybe_base) {
						has_entity_changed = true;
						removed_components[idx] = true;
						return;
					}
					else if (maybe_enco && !maybe_base) {
						component_type base_compo;
						component_type enco_compo = *maybe_enco;
		
						transform_component_ids_to_guids_in_place(enco_compo, enco);
		
						augs::write_delta(base_compo, enco_compo, new_content, true);
		
						has_entity_changed = true;
						overridden_components[idx] = true;
					}
					else {
						component_type base_compo = *maybe_base;
						component_type enco_compo = *maybe_enco;
		
						transform_component_ids_to_guids_in_place(base_compo, base);
						transform_component_ids_to_guids_in_place(enco_compo, enco);
		
						if (augs::write_delta(base_compo, enco_compo, new_content)) {
							has_entity_changed = true;
							overridden_components[idx] = true;
						}
					}
				}
			}
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
		else if (has_entity_changed) {
			augs::write(dt.stream_for_changed, stream_written_id);

			augs::write_flags(dt.stream_for_changed, overridden_components);
			augs::write_flags(dt.stream_for_changed, removed_components);
			augs::write(dt.stream_for_changed, new_content);

			++dt.changed_entities;
		}
	});

	base.significant.pool_for_aggregates.for_each_id([&base, &enco, &out, &dt](const entity_id id) {
		const const_entity_handle base_entity = base.get_handle(id);
#if COSMOS_TRACKS_GUIDS
		const auto stream_written_id = base_entity.get_guid();
		const auto maybe_enco_entity = enco.guid_to_id.find(stream_written_id);
		const bool is_dead = maybe_enco_entity == enco.guid_to_id.end();
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
	const bool reinfer_partially
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

		new_entities_ids.emplace_back(deco.create_entity_with_specific_guid(new_guid));
#else
		// otherwise new entity_id assignment needs be deterministic
#endif
	}

	for(const auto new_entity_id : new_entities_ids) {
		const auto new_entity = deco[new_entity_id];

		std::array<bool, COMPONENTS_COUNT> overridden_components;

		augs::read_flags(in, overridden_components);

		for_each_component_type(
			[&overridden_components, &new_entity, &in, &deco](auto c) {
				using component_type = decltype(c);
		
				// guid components is handled separately by the cosmos

				if constexpr(!std::is_same_v<component_type, components::guid>) {
					constexpr size_t idx = component_index_v<component_type>;
					
					if (overridden_components[idx]) {
						component_type decoded_component;
		
						augs::read_delta(decoded_component, in, true);
						transform_component_guids_to_ids_in_place(decoded_component, deco);
		
						new_entity.allocator::template add(decoded_component);
					}
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
		std::array<bool, COMPONENTS_COUNT> removed_components;
		augs::read_flags(in, overridden_components);
		augs::read_flags(in, removed_components);

		for_each_component_type(
			[&overridden_components, &removed_components, &in, &deco, &changed_entity](auto c) {
				using component_type = decltype(c);
		
				if constexpr(!std::is_same_v<component_type, components::guid>) {
					constexpr size_t idx = component_index_v<component_type>;
		
					if (overridden_components[idx]) {
						const auto maybe_component = changed_entity.allocator::template find<component_type>();
						
						if (maybe_component == nullptr) {
							component_type decoded_component;
		
							augs::read_delta(decoded_component, in, true);
							
							transform_component_guids_to_ids_in_place(decoded_component, deco);
							changed_entity.allocator::template add(decoded_component);
						}
						else {
							transform_component_ids_to_guids_in_place(*maybe_component, deco);
							augs::read_delta(*maybe_component, in);
							transform_component_guids_to_ids_in_place(*maybe_component, deco);
						}
					}
					else if (removed_components[idx]) {
						if constexpr(!is_component_fundamental_v<component_type>) {
							changed_entity.allocator::template remove<component_type>();
						}
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
