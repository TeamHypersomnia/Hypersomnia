#include "cosmic_delta.h"
#include "augs/templates/introspect.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/delta_compression.h"
#include "augs/misc/pool/pooled_object_id.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/for_each_component_type.h"

#include "augs/readwrite/byte_readwrite.h"

template <class T>
constexpr std::size_t component_index_v = index_in_list_v<T, component_list_t<type_list>>;

/* Several assumptions regarding delta encoding */

template <class T>
void transform_component_ids_to_guids_in_place(
	T& comp,
	const cosmos& cosm
) {
	augs::introspect(augs::recursive([&](auto&& self, auto, auto& id) {
		using id_type = std::decay_t<decltype(id)>;

		if constexpr(std::is_base_of_v<entity_id, id_type>) {
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
			if constexpr(is_container_v<id_type>) {
				for (auto&& e : id) {
					augs::introspect_if_not_leaf(augs::recursive(self), e);
				}
			}
			else {
				augs::introspect_if_not_leaf(augs::recursive(self), id);
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
		using id_type = std::decay_t<decltype(id)>;

		if constexpr(std::is_base_of_v<entity_id, id_type>) {
			const auto guid_inside = reinterpret_cast<const entity_guid&>(id);

			id.unset();

			if (guid_inside != 0) {
				id = cosm.solvable.get_entity_id_by(guid_inside);
			}
		}
		else {
			if constexpr(is_container_v<id_type>) {
				for (auto&& e : id) {
					augs::introspect_if_not_leaf(augs::recursive(self), e);
				}
			}
			else {
				augs::introspect_if_not_leaf(augs::recursive(self), id);
			}
		}
	}), comp);
}

struct delted_stream_of_entities {
	unsigned new_entities = 0;
	unsigned changed_entities = 0;
	unsigned deleted_entities = 0;

	augs::memory_stream stream_of_new_guids;
	augs::memory_stream stream_for_new;
	augs::memory_stream stream_for_changed;
	augs::memory_stream stream_for_deleted;
};

bool cosmic_delta::encode(
	const cosmos& base, 
	const cosmos& enco, 
	augs::memory_stream& out
) {
	auto scope = measure_scope(enco.profiler.delta_encoding);
	
	delted_stream_of_entities dt;

	enco.solvable.significant.entity_pool.for_each_id([&](const entity_id id) {
		const const_entity_handle enco_entity = enco[id];
		const auto stream_written_guid = enco_entity.get_guid();
		const const_entity_handle base_entity = base[base.solvable.get_entity_id_by(stream_written_guid)];
		const bool is_new = base_entity.dead();

		bool has_entity_changed = false;

		std::array<bool, COMPONENTS_COUNT> overridden_components;
		std::array<bool, COMPONENTS_COUNT> removed_components;
		std::fill(overridden_components.begin(), overridden_components.end(), false);
		std::fill(removed_components.begin(), removed_components.end(), false);

		augs::memory_stream new_content;
		
		for_each_component_type(
			[&base, &enco, base_entity, is_new, enco_entity, &has_entity_changed, &new_content, &overridden_components, &removed_components](auto c) {
				using component_type = decltype(c);
		
				// guid components is handled separately by the cosmos

				if constexpr(!std::is_same_v<component_type, components::guid>) {
					constexpr size_t idx = component_index_v<component_type>;
		
					const auto maybe_base = is_new ? nullptr : base_entity.get().find<component_type>(base.solvable);
					const auto maybe_enco = enco_entity.get().find<component_type>(enco.solvable);
		
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
			augs::write_bytes(dt.stream_of_new_guids, stream_written_guid);

			augs::write_flags(dt.stream_for_new, overridden_components);
			dt.stream_for_new.write(new_content);

			++dt.new_entities;
		}
		else if (has_entity_changed) {
			augs::write_bytes(dt.stream_for_changed, stream_written_guid);

			augs::write_flags(dt.stream_for_changed, overridden_components);
			augs::write_flags(dt.stream_for_changed, removed_components);
			dt.stream_for_changed.write(new_content);

			++dt.changed_entities;
		}
	});

	base.solvable.significant.entity_pool.for_each_id([&base, &enco, &dt](const entity_id id) {
		const const_entity_handle base_entity = base[id];
		const auto stream_written_guid = base_entity.get_guid();
		const auto maybe_enco_entity = enco.solvable.get_entity_id_by(stream_written_guid);
		const bool is_dead = !maybe_enco_entity.is_set();

		if (is_dead) {
			++dt.deleted_entities;
			augs::write_bytes(dt.stream_for_deleted, stream_written_guid);
		}
	});

	augs::memory_stream new_meta_content;

	const bool has_meta_changed = augs::write_delta(
		base.solvable.significant.meta, 
		enco.solvable.significant.meta, 
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
		augs::write_bytes(out, true);

		out.write(new_meta_content);

		augs::write_bytes(out, dt.new_entities);
		augs::write_bytes(out, dt.changed_entities);
		augs::write_bytes(out, dt.deleted_entities);

		out.write(dt.stream_of_new_guids);
		out.write(dt.stream_for_new);
		out.write(dt.stream_for_changed);
		out.write(dt.stream_for_deleted);
	}
	else {
		augs::write_bytes(out, false);
	}

	enco.profiler.delta_bytes.measure(out.size());
	base.profiler.delta_bytes.measure(out.size());

	return has_anything_changed;
}

void cosmic_delta::decode(
	cosmos& deco, 
	augs::memory_stream& in, 
	const bool reinfer_partially
) {
	if (in.get_unread_bytes() == 0) {
		return;
	}

	bool has_anything_changed = false;

	augs::read_bytes(in, has_anything_changed);

	if (!has_anything_changed) {
		return;
	}
	
	deco.profiler.delta_decoding.start();

	deco.solvable.destroy_all_caches();

	augs::read_delta(deco.solvable.significant.meta, in, true);

	delted_stream_of_entities dt;

	augs::read_bytes(in, dt.new_entities);
	augs::read_bytes(in, dt.changed_entities);
	augs::read_bytes(in, dt.deleted_entities);

	size_t new_guids = dt.new_entities;
	std::vector<entity_id> new_entities_ids;

	while (dt.new_entities--) {
		entity_guid new_guid;

		augs::read_bytes(in, new_guid);
		
		new_entities_ids.push_back(deco.create_entity_with_specific_guid(new_guid).get_id());
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
		
						new_entity.get().add(decoded_component, deco.solvable);
					}
				}
			}
		);
	}

	while (dt.changed_entities--) {
		entity_guid guid_of_changed;
		
		augs::read_bytes(in, guid_of_changed);
		
		const auto changed_entity = deco[guid_of_changed];

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
						const auto maybe_component = changed_entity.get().find<component_type>(deco.solvable);
						
						if (maybe_component == nullptr) {
							component_type decoded_component;
		
							augs::read_delta(decoded_component, in, true);
							
							transform_component_guids_to_ids_in_place(decoded_component, deco);
							changed_entity.get().add(decoded_component, deco.solvable);
						}
						else {
							transform_component_ids_to_guids_in_place(*maybe_component, deco);
							augs::read_delta(*maybe_component, in);
							transform_component_guids_to_ids_in_place(*maybe_component, deco);
						}
					}
					else if (removed_components[idx]) {
						if constexpr(!is_component_fundamental_v<component_type>) {
							changed_entity.get().remove<component_type>(deco.solvable);
						}
					}
				}
			}
		);
	}

	while (dt.deleted_entities--) {
		entity_guid guid_of_destroyed;

		augs::read_bytes(in, guid_of_destroyed);
		
		deco.delete_entity(deco[guid_of_destroyed]);
	}

	const auto unread_bits = in.get_unread_bytes();
	//should_eq(0, unread_bits);

	deco.infer_all_caches();

	deco.profiler.delta_decoding.stop();
}
