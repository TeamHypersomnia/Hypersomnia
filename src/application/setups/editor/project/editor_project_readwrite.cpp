#include <unordered_set>
#include "augs/string/path_sanitization.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/detail/is_editor_typed_resource.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/detail/is_editor_typed_node.h"
#include "application/setups/editor/detail/has_reference_count.h"
#include "application/setups/editor/resources/resource_traits.h"
#include "augs/image/image.h"
#include "augs/templates/in_order_of.h"

namespace augs {
	template <class T, class R>
	void to_json_value(T& out, const editor_typed_resource_id<R>& from) {
		if (from.is_set()) {
			out.String(from._serialized_resource_name);
		}
		else {
			out.Null();
		}
	}

	template <class T, class R>
	void from_json_value(T& from, editor_typed_resource_id<R>& out) {
		if (from.IsString()) {
			out._serialized_resource_name = from.GetString();

			if (out._serialized_resource_name.empty()) {
				out = {};
			}
		}

		if (from.IsNull() || (from.IsBool() && !from.GetBool())) {
			out = {};
		}
	}

	template <class T, class R>
	void to_json_value(T& out, const editor_typed_node_id<R>& from) {
		if (from.is_set()) {
			out.String(from._serialized_node_name);
		}
		else {
			out.Null();
		}
	}

	template <class T, class R>
	void from_json_value(T& from, editor_typed_node_id<R>& out) {
		if (from.IsString()) {
			out._serialized_node_name = from.GetString();

			if (out._serialized_node_name.empty()) {
				out = {};
			}
		}

		if (from.IsNull() || (from.IsBool() && !from.GetBool())) {
			out = {};
		}
	}
}

#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/project/editor_project.hpp"

namespace augs {
	/* 
		This is ugly-ass but will only be used for layer id i/o,
		layers can be serialized first because they have no dependencies
	*/

	auto*& written_editor_project() {
		thread_local const editor_project* project = nullptr;
		return project;
	}

	auto*& read_editor_project() {
		thread_local editor_project* project = nullptr;
		return project;
	}

	template <class T>
	void to_json_value(T& out, const editor_layer_id& from) {
		if (from.is_set()) {
			if (const auto layer = written_editor_project()->find_layer(from)) {
				out.String(layer->unique_name.c_str());
			}
		}
		else {
			out.Null();
		}
	}

	template <class T>
	void from_json_value(T& from, editor_layer_id& out) {
		if (from.IsString()) {
			out = read_editor_project()->find_layer_id(from.GetString());
		}

		if (from.IsNull() || (from.IsBool() && !from.GetBool())) {
			out = {};
		}
	}

}

#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/editor_official_resource_map.hpp"
#include "application/setups/editor/defaults/editor_game_mode_defaults.h"
#include "application/setups/editor/defaults/editor_project_defaults.h"

#include "augs/misc/pool/pool_allocate.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/templates/introspection_utils/on_each_object_in_object.h"
#include "application/setups/editor/create_name_to_id_map.hpp"

#include "application/setups/editor/editor_filesystem_node_type.h"
#include "application/setups/editor/defaults/editor_resource_defaults.h"
#include "application/setups/editor/defaults/editor_node_defaults.h"
#include "application/setups/editor/project/on_each_resource_id_in_project.hpp"
#include "application/setups/editor/project/on_each_node_id_in_project.hpp"

#if 0
template <class R>
void unpack_string_id(resource_name_to_id& ids, editor_typed_resource_id<R>& id) {
	if (auto found = mapped_or_nullptr(ids.get_for<R>(), id._serialized_resource_name)) {
		id = *found;
	}
}
#else
template <class R>
void unpack_string_id(resource_name_to_id& ids, editor_typed_resource_id<R>& id, const bool strict) {
	if (id._serialized_resource_name.empty()) {
		return;
	}

	if (auto found = mapped_or_nullptr(ids, id._serialized_resource_name)) {
		if ((*found).type_id.template is<R>()) {
			id = editor_typed_resource_id<R>::from_generic(*found);
		}
		else {
			if (strict) {
				throw augs::json_deserialization_error(
					"Invalid resource property:\n\"%x\" is not a %x!",
					id._serialized_resource_name,
					R::get_type_name()
				);
			}
		}
	}
	else {
		if (strict) {
			throw augs::json_deserialization_error(
				"Invalid resource property: \"%x\"\nResource not found!",
				id._serialized_resource_name
			);
		}
	}

	id._serialized_resource_name.clear();
}

using modes_map = std::unordered_map<std::string, editor_typed_resource_id<editor_game_mode_resource>>;

void unpack_string_id(const modes_map& modes, editor_typed_resource_id<editor_game_mode_resource>& id, const bool strict) {
	if (id._serialized_resource_name.empty()) {
		return;
	}

	if (auto found = mapped_or_nullptr(modes, id._serialized_resource_name)) {
		id = *found;
	}
	else {
		if (strict) {
			throw augs::json_deserialization_error(
				"Invalid mode property: \"%x\"\nMode not found!",
				id._serialized_resource_name
			);
		}
	}

	id._serialized_resource_name.clear();
}

template <class M, class N>
void unpack_string_id(const M& node_name_map, editor_typed_node_id<N>& id, const bool strict) {
	if (id._serialized_node_name.empty()) {
		return;
	}

	if (auto found = mapped_or_nullptr(node_name_map, id._serialized_node_name)) {
		id = editor_typed_node_id<N>::from_generic(found->id);
	}
	else {
		if (strict) {
			throw augs::json_deserialization_error(
				"Invalid node property: \"%x\"\nNode not found!",
				id._serialized_node_name
			);
		}
	}

	id._serialized_node_name.clear();
}
#endif

template <class T, class F>
std::optional<T> GetIf(F& from, const std::string& label) {
	if (from.HasMember(label) && from[label].template Is<T>()) {
		return from[label].template Get<T>();
	}

	return std::nullopt;
}

template <class F>
auto FindArray(F& from, const std::string& label) -> std::optional<decltype(std::declval<F>()[std::declval<std::string>()].GetArray())> {
	if (from.HasMember(label) && from[label].IsArray()) {
		return from[label].GetArray();
	}

	return std::nullopt;
}

template <class F>
auto FindObject(F& from, const std::string& label) -> std::optional<decltype(std::declval<F>()[std::declval<std::string>()].GetObject())> {
	if (from.HasMember(label) && from[label].IsObject()) {
		return from[label].GetObject();
	}

	return std::nullopt;
}

bool editor_project::recount_references(const O& officials, const bool recount_officials) const {
	bool any_pathed_project_resources_referenced = false;

	auto clear_refs = [&]<typename P>(const P& pool) {
		using resource_type = typename P::mapped_type;

		for (auto& resource : pool) {
			if constexpr(has_reference_count_v<resource_type>) {
				resource.reference_count = 0;
			}

			if constexpr(has_changes_detected_v<resource_type>) {
				resource.changes_detected = false;
			}
		}
	};

	if (recount_officials) {
		officials.pools.for_each_container(clear_refs);
	}

	resources.pools.for_each_container(clear_refs);

	auto count_refs = [&]<typename R>(const editor_typed_resource_id<R>& id) {
		if constexpr(has_reference_count_v<R>) {
			if (id.is_official && !recount_officials) {
				return;
			}

			if (const auto resource = find_resource(officials, id)) {
				if (!id.is_official) {
					if constexpr(is_pathed_resource_v<R>) {
						any_pathed_project_resources_referenced = true;
					}
				}

				resource->reference_count += 1;
			}
		}
	};

	on_each_resource_id_in_project(*this, count_refs);

	return any_pathed_project_resources_referenced;
}

bool editor_project::mark_changed_resources(const editor_official_resource_map& officials_map) const {
	bool any_pathed_project_resources_changed = false;

	auto mark_changed = [&]<typename P>(const P& pool) {
		using resource_type = typename P::mapped_type;

		if constexpr(has_changes_detected_v<resource_type>) {
			auto defaults = decltype(resource_type::editable)();
			::setup_resource_defaults(defaults, officials_map);

			/*
				This will skip writing unused resources for which only size has changed.
				When an image changes its size on disk while editor still works,
				the old size could wrongly be interpreted as one that is different from the new default.
			*/

			static constexpr bool is_sprite = std::is_same_v<resource_type, editor_sprite_resource>;

			if constexpr(is_sprite) {
				defaults.size = vec2i::zero;
			}

			for (auto& resource : pool) {
				auto compared = resource.editable;

				if constexpr(is_sprite) {
					compared.size = vec2i::zero;
				}

				const bool changed = !(compared == defaults);
				resource.changes_detected = changed;

				if (changed) {
					if constexpr(is_pathed_resource_v<resource_type>) {
						any_pathed_project_resources_changed = true;
					}
				}
			}
		}
	};

	resources.pools.for_each_container(mark_changed);

	return any_pathed_project_resources_changed;
}

namespace editor_project_readwrite {
	void write_editor_view(const augs::path_type& json_path, const editor_view& view) {
		augs::save_as_json(view, json_path);
	}

	editor_view read_editor_view(const augs::path_type& json_path) {
		return augs::from_json_file<editor_view>(json_path);
	}

	void write_project_json(
		const augs::path_type& json_path,
		const editor_project& project,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map
	) {
		/* 
			Scan before stringifying resource ids.
			If we scanned later, mark_changed_resources could return false positives,
			because ids are compared by stringified ids when either is set.
		*/

		/* This pointer will only be used for layer id i/o. */
		augs::written_editor_project() = std::addressof(project);

		const bool any_external_resources_to_track = project.rescan_resources_to_track(officials, officials_map);

		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
		//writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);

		editor_project project_defaults;

		auto taken_pseudoids = resource_name_to_id();

		auto resolve_pseudoids_of_project_resources = [&]() {
			/*
				Given that we now know that external resources, internal resources and official resources
				can only ever collide within each respective group,
				(because we're preffixing them with @ and !)
				we can only check externals against externals.
			*/

			auto resolve_pseudoids = [&]<typename P>(const P& pool) {
				using R = typename P::mapped_type;

				if constexpr(is_pathed_resource_v<R>) {
					pool.for_each_id_and_object(
						[&](const auto raw_id, const auto& typed_resource) {
							const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, false);
							const auto& path_in_project = typed_resource.external_file.path_in_project;
							const auto short_pseudoid = "@" + path_in_project.stem().string();

							const auto result = taken_pseudoids.try_emplace(short_pseudoid, typed_id.operator editor_resource_id());

							if (const bool duplicate = !result.second) {
								auto make_certainly_unique_pseudoid = [&]<typename O>(const O& of) {
									/* Have to if constexpr again because we're dispatching on the generic editor_resource_id */
									if constexpr(is_pathed_resource_v<O>) {
										/* Guaranteed to be unique */
										of.resolved_pseudoid = "@" + of.external_file.path_in_project.string();
									}
								};

								make_certainly_unique_pseudoid(typed_resource);

								project.on_resource(
									officials,
									(*result.first).second,
									[make_certainly_unique_pseudoid](const auto& typed_duplicate, const auto) {
										make_certainly_unique_pseudoid(typed_duplicate);
									}
								);
							}
							else {
								typed_resource.resolved_pseudoid = short_pseudoid;
							}
						}
					);
				}
			};

			project.resources.pools.for_each_container(resolve_pseudoids);
		};

		auto get_pseudoid = [&]<typename R>(const editor_typed_resource_id<R>& resource_id) {
			const auto resource = project.find_resource(officials, resource_id);

			if (resource) {
				if constexpr(is_pathed_resource_v<R>) {
					if (resource_id.is_official) {
						return resource->cached_official_name;
					}
					else {
						return resource->resolved_pseudoid;
					}
				}
				else {
					if (resource_id.is_official) {
						return resource->unique_name;
					}
					else {
						if constexpr(std::is_same_v<R, editor_game_mode_resource>) {
							return resource->unique_name;
						}
						else {
							return "!" + resource->unique_name;
						}
					}
				}
			}

			return std::string("");
		};

		auto clean_stringified_resource_ids = [](auto& subject) {
			auto clean = [&](auto& id) {
				id._serialized_resource_name.clear();
			};

			::on_each_resource_id_in_project(subject, clean);
		};

		auto stringify_resource_id = [&get_pseudoid](auto& typed_id) {
			typed_id._serialized_resource_name = get_pseudoid(typed_id);
		};

		auto stringify_resource_ids = [&stringify_resource_id](const editor_project& subject) {
			::on_each_resource_id_in_project(subject, stringify_resource_id);
		};

		auto clean_stringified_node_ids = [](const editor_project& subject) {
			auto clean = [&](auto& id) {
				id._serialized_node_name.clear();
			};

			::on_each_node_id_in_project(subject, clean);
		};

		auto stringify_node_id = [&project](auto& typed_id) {
			if (const auto node = project.find_node(typed_id)) {
				typed_id._serialized_node_name = node->unique_name;
			}
			else {
				typed_id._serialized_node_name.clear();
			}
		};

		auto stringify_node_ids = [&stringify_node_id](const editor_project& subject) {
			::on_each_node_id_in_project(subject, stringify_node_id);
		};

		auto setup_project_defaults = [&]() {
			/*
				We do this so we don't redundantly write default values. 
				The written editor project will then be compared against this one.
			*/

			auto& defaults = project_defaults;

			auto initialize_project_structs = [&]() {
				::setup_project_defaults(defaults.settings, defaults.get_game_modes(), officials_map);
				::setup_project_defaults(defaults.playtesting, defaults.get_game_modes(), officials_map);
			};

			initialize_project_structs();
			defaults.playtesting.mode._serialized_resource_name = "quick_test";
		};

		auto write_project_structs = [&]() {
			augs::write_json_diff(writer, project, project_defaults);
		};

		auto write_materials = [&]() {
			const auto& materials = project.resources.get_pool_for<editor_material_resource>();

			if (materials.empty()) {
				return;
			}

			writer.Key("materials");
			writer.StartArray();

			const auto defaults = editor_material_resource_editable();

			auto write = [&](const auto& material) {
				const auto& editable = material.editable;

				writer.StartObject();
				writer.Key("id");
				writer.String(material.unique_name);

				augs::write_json_diff(writer, editable, defaults);

				writer.EndObject();
			};

			::in_order_of(
				materials,
				[&](const auto& material) {
					return material.unique_name;
				},
				[&](const auto& a, const auto& b) {
					return augs::natural_order(a, b);
				},
				write
			);

			writer.EndArray();
		};

		auto write_modes = [&]() {
			const auto& modes = project.get_game_modes();

			if (modes.empty()) {
				return;
			}

			auto defaults = editor_game_mode_resource();
			::setup_game_mode_defaults(defaults.editable, officials_map);
			::on_each_resource_id_in_object(defaults.editable, stringify_resource_id);

			bool all_default = true;

			for (auto& m : modes) {
				if (!(m.editable == defaults.editable)) {
					all_default = false;
				}
			}

			if (all_default) {
				return;
			}

			writer.Key("game_modes");
			writer.StartObject();

			auto write = [&](const auto& mode) {
				if (mode.editable == defaults.editable) {
					return;
				}

				writer.Key(mode.unique_name);
				augs::write_json_diff(writer, mode.editable, defaults.editable, true);
			};

			::in_order_of(
				modes,
				[&](const auto& mode) {
					return mode.unique_name;
				},
				[&](const auto& a, const auto& b) {
					return augs::natural_order(a, b);
				},
				write
			);

			writer.EndObject();
		};

		auto write_layers = [&]() {
			const auto& layers = project.layers.order;

			if (layers.empty()) {
				return;
			}

			writer.Key("layers");
			writer.StartArray();

			auto defaults = editor_layer();

			for (const auto& layer_id : layers) {
				if (const auto layer = project.find_layer(layer_id)) {
					writer.StartObject();

					writer.Key("id");
					writer.String(layer->unique_name);

					if (layer->is_open != defaults.is_open) {
						writer.Key("is_open_in_editor");
						writer.Bool(layer->is_open);
					}

					augs::write_json_diff(writer, layer->editable, defaults.editable);

					if (!layer->hierarchy.nodes.empty()) {
						writer.Key("nodes");

						writer.StartArray();

						for (auto& node_id : layer->hierarchy.nodes) {
							project.on_node(node_id, [&](const auto& typed_node, const auto) {
								writer.String(typed_node.unique_name);
							});
						}

						writer.EndArray();
					}

					writer.EndObject();
				}
			}

			writer.EndArray();
		};

		auto write_external_resources = [&]() {
			if (!any_external_resources_to_track) {
				return;
			}

			writer.Key("external_resources");
			writer.StartArray();

			auto sort_and_write = [&]<typename P>(const P& pool) {
				using R = typename P::mapped_type;

				auto defaults = decltype(R::editable)();
				::setup_resource_defaults(defaults, officials_map);
				::on_each_resource_id_in_object(defaults, stringify_resource_id);

				if constexpr(is_pathed_resource_v<R>) {
					auto write = [&](const auto& typed_resource) {
						const bool should_write = typed_resource.should_be_tracked();

						if (!should_write) {
							return;
						}

						const auto& editable = typed_resource.editable;
						const auto& file = typed_resource.external_file;

						writer.StartObject();
						writer.Key("path");
						augs::general_write_json_value(writer, file.path_in_project);
						writer.Key("file_hash");
						writer.String(file.file_hash);
						writer.Key("id");
						writer.String(typed_resource.resolved_pseudoid);

						augs::write_json_diff(writer, editable, defaults);

						writer.EndObject();
					};

					::in_order_of(
						pool,
						[&](const auto& typed_resource) {
							return typed_resource.external_file.path_in_project.string();
						},
						[&](const auto& a, const auto& b) {
							return augs::natural_order(a, b);
						},
						write
					);
				}
			};

			project.resources.pools.for_each_container(sort_and_write);

			writer.EndArray();
		};

		auto write_nodes = [&]() {
			{
				bool any_nodes = false;

				project.nodes.pools.for_each_container([&](const auto& pool) {
					if (!pool.empty()) {
						any_nodes = true;
					}
				});

				if (!any_nodes) {
					return;
				}
			}

			writer.Key("nodes");
			writer.StartArray();

			/*
				Reuse vector of sorted entries across node types,
				as the sorted entry is exactly the same type across node types.
			*/

			struct sorted_entry {
				uint32_t chronological_order = 0;
				uint32_t index = 0;
			};

			thread_local std::vector<sorted_entry> sorted;

			/*
				We iterate in reverse so that sprites will be serialized last.
				(most new nodes will then be appended to the end of file) 
			*/
			project.nodes.pools.for_each_container_reverse([&]<typename P>(const P& pool) {
				using node_type = typename P::mapped_type;
				auto defaults = node_type();

				auto write = [&](const node_type& typed_node) {
					if (const auto resource = project.find_resource(officials, typed_node.resource_id)) {
						writer.StartObject();

						/* Already stringified with a call to stringify_resource_ids */
						const auto pseudoid = typed_node.resource_id._serialized_resource_name;

						writer.Key("id");
						writer.String(typed_node.unique_name);

						writer.Key("type");
						writer.String(pseudoid);

						if (typed_node.active != defaults.active) {
							writer.Key("active");
							writer.Bool(typed_node.active);
						}

						::setup_node_defaults(defaults.editable, *resource);
						::on_each_resource_id_in_object(defaults.editable, stringify_resource_id);

						/* 
							Force position to always be written,
							even if it's default at [0, 0].
						*/

						defaults.editable.pos = typed_node.editable.pos + vec2(1, 1);

						augs::write_json_diff(writer, typed_node.editable, defaults.editable);

						writer.EndObject();
					}
				};

				::in_order_of(
					pool,
					[&](const auto& typed_node) {
						return typed_node.chronological_order;
					},
					std::less<uint32_t>(),
					write
				);
			});


			writer.EndArray();
		};

		setup_project_defaults();

		resolve_pseudoids_of_project_resources();

		stringify_resource_ids(project_defaults);
		stringify_resource_ids(project);
		stringify_node_ids(project);

		writer.StartObject();

		write_project_structs();
		write_modes();

		write_external_resources();
		write_materials();

		write_layers();
		write_nodes();

		writer.EndObject();

		/*
			Layer hierarchies and nodes are ignored when writing.
			We have to write them manually.
		*/

		augs::save_as_text(json_path, s.GetString());

		clean_stringified_resource_ids(project);
		clean_stringified_node_ids(project);
	}

	editor_project read_project_json(
		const augs::path_type& json_path,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map,
		const reading_settings settings,
		augs::secure_hash_type* const output_arena_hash
	) {
		const auto project_dir = json_path.parent_path();

		const auto read_string = output_arena_hash != nullptr ? augs::file_to_string_crlf_to_lf : augs::file_to_string;

		return read_project_json(project_dir, read_string(json_path), officials, officials_map, settings, output_arena_hash);
	}

	editor_project read_project_json(
		const augs::path_type& project_dir,
		const std::string& loaded_project_json,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map,
		const reading_settings settings,
		augs::secure_hash_type* const output_arena_hash
	) {
		const bool strict = settings.strict;
		const auto document = augs::json_document_from(loaded_project_json);

		editor_project loaded;

		/* This pointer will only be used for layer id i/o. */
		augs::read_editor_project() = std::addressof(loaded);

		auto resource_map = officials_map.create_name_to_id_map();
		auto mode_map = modes_map();

		auto register_new_resource = [&](const auto name_id, auto& allocation_result) {
			auto& allocated = allocation_result.object;

			using O = remove_cref<decltype(allocated)>;
			using Id = editor_typed_resource_id<O>;

			const auto typed_id = Id::from_raw(allocation_result.key, false);

			if constexpr(std::is_same_v<O, editor_game_mode_resource>) {
				const auto result = mode_map.try_emplace(name_id, typed_id);

				if (strict) {
					if (!result.second) {
						throw augs::json_deserialization_error("Duplicate mode: \"%x\"", name_id);
					}
				}
			}
			else {
				const auto result = resource_map.try_emplace(name_id, typed_id.operator editor_resource_id());

				if (strict) {
					if (!result.second) {
						throw augs::json_deserialization_error("Duplicate resource: \"%x\"", name_id);
					}
				}
			}
		};

		auto initialize_project_structs = [&]() {
			::setup_project_defaults(loaded.settings, loaded.get_game_modes(), officials_map);

			/* 
				This doesn't set the playtesting mode (done later below) 
				but maybe there will be other important properties in the future
			*/

			::setup_project_defaults(loaded.playtesting, loaded.get_game_modes(), officials_map);
		};

		auto read_project_structs = [&]() {

			/*
				Will leave out non-trivial fields like layers, nodes and resources.
				(see: static constexpr bool json_ignore = true)
				Only stuff like about, meta, settings etc. get loaded in this call.
			*/

			static_assert(augs::has_custom_to_json_value_v<decltype(editor_playtesting_settings::mode)>);

			augs::read_json(document, loaded);
		};

		auto read_materials = [&]() {
			const auto maybe_materials = FindArray(document, "materials");

			if (!maybe_materials) {
				return;
			}

			for (auto& resource : *maybe_materials) {
				if (!resource.IsObject()) {
					continue;
				}

				if (!resource.HasMember("id") || !resource["id"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"id\" property for a material!");
					}

					continue;
				}

				const auto id = resource["id"].GetString();

				editor_material_resource new_material;
				new_material.unique_name = id;

				augs::read_json(resource, new_material.editable);

				auto& pool = loaded.resources.get_pool_for<editor_material_resource>();
				const auto result = pool.allocate(std::move(new_material));
				register_new_resource(std::string("!") + id, result);
			}
		};

		auto read_modes = [&]() {
			const auto maybe_modes = FindObject(document, "game_modes");

			if (!maybe_modes) {
				return;
			}

			auto& modes = loaded.get_game_modes();

			for (auto& mode : *maybe_modes) {
				if (!mode.value.IsObject()) {
					continue;
				}

				const auto key = std::string(mode.name.GetString());

				editor_game_mode_resource new_game_mode;
				::setup_game_mode_defaults(new_game_mode.editable, officials_map);

				augs::read_json(mode.value, new_game_mode.editable.common);

				auto try_read = [&]<typename M>(M& into) {
					if (key == M::get_identifier()) {
						new_game_mode.type.set<M>();
						augs::read_json(mode.value, into);
						return true;
					}

					return false;
				};

				if (try_read(new_game_mode.editable.quick_test)) {

				}
				else if (try_read(new_game_mode.editable.bomb_defusal)) {
					
				}
				else if (try_read(new_game_mode.editable.gun_game)) {

				}
				else {
					if (strict) {
						throw augs::json_deserialization_error("Unknown game mode: \"%x\"", key);
					}

					continue;
				}

				new_game_mode.unique_name = key;

				const auto result = modes.allocate(std::move(new_game_mode));
				register_new_resource(key, result);
			}
		};

		auto read_external_resources = [&]() {
			std::unordered_set<augs::path_type> existing;

			const auto maybe_externals = FindArray(document, "external_resources");

			if (!maybe_externals) {
				return;
			}

			for (auto& resource : *maybe_externals) {
				if (!resource.IsObject()) {
					continue;
				}

				if (!resource.HasMember("path") || !resource["path"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"path\" property for an external resource!");
					}

					continue;
				}

				if (!resource.HasMember("file_hash") || !resource["file_hash"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"file_hash\" property for %x!", resource["path"].GetString());
					}

					continue;
				}

				if (!resource.HasMember("id") || !resource["id"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"id\" property for %x!", resource["path"].GetString());
					}

					continue;
				}

				const auto id = resource["id"].GetString();
				const auto file_hash = resource["file_hash"].GetString();

				auto load_resource = [&](const augs::path_type& path) {
					if (found_in(existing, path)) {
						if (strict) {
							throw augs::json_deserialization_error("Duplicate entries for %x!", path);
						}

						return;
					}

					existing.emplace(path);

					/*
						Determine type based on the extension.
					*/

					auto read_as = [&]<typename R>(R typed_resource) {
						if constexpr(std::is_same_v<R, editor_sprite_resource>) {
							if (path.extension() == ".gif") {
								typed_resource.animation_frames = augs::image::read_gif_frame_meta(project_dir / path);
							}
							else {
								typed_resource.animation_frames.clear();
							}
						}

						::setup_resource_defaults(typed_resource.editable, officials_map);
						augs::read_json(resource, typed_resource.editable);

						auto& pool = loaded.resources.pools.get_for<R>();
						const auto result = pool.allocate(std::move(typed_resource));
						register_new_resource(id, result);
					};

					const auto extension = path.extension();
					const auto type = ::get_filesystem_node_type_by_extension(extension);

					using Type = editor_filesystem_node_type;

					const auto pathed = editor_pathed_resource(
						/*
							In case of the editor,
							these two props are read basically only so that the editor can later 
							associate the paths/hashes to existing files - 
							since the editor on its own iterates the entire project folder in search of paths.
						*/

						path,
						file_hash,
						{}
					);

					switch (type) {
						case Type::IMAGE:
							read_as(editor_sprite_resource(pathed));
							break;
						case Type::SOUND:
							read_as(editor_sound_resource(pathed));
							break;

						default: 
							if (strict) {
								throw augs::json_deserialization_error("Failed to load %x: unknown extension!", path);
							}

							return;
					}
				};

				const auto untrusted_path = resource["path"].GetString();

				std::visit(
					[&]<typename R>(const R& result) {
						if constexpr(std::is_same_v<R, sanitization::forbidden_path_type>) {
							const auto err = typesafe_sprintf(
								"Failed to load %x:\n%x",
								untrusted_path,
								sanitization::describe(result)
							);

							if (strict) {
								throw augs::json_deserialization_error(err);
							}
						}
						else if constexpr(std::is_same_v<R, augs::path_type>) {
							load_resource(result);
						}
						else {
							static_assert(always_false_v<R>, "Non-exhaustive if constexpr");
						}
					},

					sanitization::sanitize_downloaded_file_path(project_dir, untrusted_path)
				);
			}
		};

		struct node_meta {
			editor_node_id id = editor_node_id();
			bool assigned_to_layer = false;
		};

		std::unordered_map<std::string, node_meta> node_name_map;
		std::unordered_set<std::string> skipped_nodes;

		const bool create_fallback_node_order = [&]() {
			auto layers = FindArray(document, "layers");

			if (!layers) {
				return true;
			}

			if (layers->Size() > 1) {
				return false;
			}

			for (auto& layer : *layers) {
				const auto maybe_nodes = FindArray(layer, "nodes");

				if (maybe_nodes) {
					return false;
				}
			}

			return true;
		}();
			
		std::vector<editor_node_id> fallback_node_order;

		auto read_nodes = [&]() {
			const auto maybe_nodes = FindArray(document, "nodes");

			if (maybe_nodes.has_value()) {
				for (auto& json_node : *maybe_nodes) {
					if (!json_node.IsObject()) {
						continue;
					}

					const auto maybe_id = GetIf<std::string>(json_node, "id");

					if (maybe_id == std::nullopt || *maybe_id == "") {
						if (strict) {
							throw augs::json_deserialization_error("Missing \"id\" property for node!");
						}

						continue;
					}

					const auto& id = *maybe_id;

					const auto maybe_type = GetIf<std::string>(json_node, "type");

					if (maybe_type == std::nullopt) {
						if (strict) {
							throw augs::json_deserialization_error("Missing \"type\" property for node \"%x\"!", id);
						}

						continue;
					}

					const auto& type = *maybe_type;
					const auto resource_id = mapped_or_nullptr(resource_map, type);

					if (resource_id == nullptr) {
						if (strict) {
							throw augs::json_deserialization_error("Invalid \"type\" property for node \"%x\".\nResource \"%x\" not found!", id, type);
						}

						continue;
					}

					auto create_node_from_resource = [&]<typename R>(const R& typed_resource, const auto typed_resource_id) {
						if constexpr(!can_be_instantiated_v<R>) {
							if (strict) {
								throw augs::json_deserialization_error(
									"Invalid \"type\" property for node \"%x\".\n\"%x\" cannot be instantiated!", id, type
								);
							}

							return;
						}
						else {
							using node_type = typename R::node_type;

							if (!settings.read_inactive_nodes) {
								if (const auto maybe_active = GetIf<bool>(json_node, "active")) {
									if (!*maybe_active) {
										skipped_nodes.emplace(id);
										return;
									}
								}
								else if (const bool inactive_by_default = !node_type().active) {
									skipped_nodes.emplace(id);
									return;
								}
							}

							const auto map_result = node_name_map.try_emplace(id, node_meta());

							if (const bool is_unique = map_result.second) {
								auto& pool = loaded.nodes.pools.get_for<node_type>();

								const auto [new_raw_id, new_node] = pool.allocate();

								if (const auto maybe_active = GetIf<bool>(json_node, "active")) {
									new_node.active = *maybe_active;
								}

								new_node.resource_id = typed_resource_id;
								new_node.unique_name = id;

								new_node.chronological_order = loaded.nodes.next_chronological_order++;

								::setup_node_defaults(new_node.editable, typed_resource);
								augs::read_json(json_node, new_node.editable);

								const auto new_typed_id = editor_typed_node_id<node_type>::from_raw(new_raw_id);
								const auto new_id = new_typed_id.operator editor_node_id();
								(*map_result.first).second.id = new_id;

								if (create_fallback_node_order) {
									fallback_node_order.push_back(new_id);
								}
							}
							else {
								if (strict) {
									throw augs::json_deserialization_error(
										"Duplicate nodes: \"%x\"!", id
									);
								}
							}
						}
					};

					loaded.on_resource(
						officials,
						*resource_id,
						create_node_from_resource
					);
				}
			}
		};

		auto read_layers = [&]() {
			auto nodes_registered = std::size_t(0);

			const auto maybe_layers = FindArray(document, "layers");

			if (!maybe_layers) {
				if (fallback_node_order.empty()) {
					return;
				}

				editor_layer layer;
				layer.unique_name = "Layer 1";
				layer.hierarchy.nodes = std::move(fallback_node_order);

				nodes_registered = layer.hierarchy.nodes.size();

				const editor_layer_id layer_id = loaded.layers.pool.allocate(std::move(layer));
				loaded.layers.order.push_back(layer_id);

				return;
			}

			for (auto& json_layer : *maybe_layers) {
				const auto id = GetIf<std::string>(json_layer, "id");

				if (id == std::nullopt) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"id\" property for layer!");
					}

					continue;
				}

				if (!settings.read_inactive_nodes) {
					if (const auto maybe_active = GetIf<bool>(json_layer, "active")) {
						if (!*maybe_active) {
							continue;
						}
					}
					else if (const bool inactive_by_default = !editor_layer_editable().active) {
						continue;
					}
				}

				editor_layer layer;
				layer.unique_name = *id;

				if (const auto is_open = GetIf<bool>(json_layer, "is_open_in_editor")) {
					layer.is_open = *is_open;
				}

				augs::read_json(json_layer, layer.editable);

				auto read_layer_nodes = [&]() {
					if (const auto maybe_nodes = FindArray(json_layer, "nodes")) {
						for (auto& layer_node : *maybe_nodes) {
							if (layer_node.IsString()) {
								const auto node_id = layer_node.GetString();
								
								if (const auto found_node = mapped_or_nullptr(node_name_map, node_id)) {
									if (found_node->assigned_to_layer) {
										if (strict) {
											throw augs::json_deserialization_error(
												"Error reading layer \"%x\":\nnode \"%x\" is already assigned to another layer!", 
												layer.unique_name,
												node_id
											);
										}

										continue;
									}
									else {
										found_node->assigned_to_layer = true;
										layer.hierarchy.nodes.push_back(found_node->id);

										++nodes_registered;
									}
								}
								else {
									if (!found_in(skipped_nodes, node_id) ) {
										if (strict) {
											throw augs::json_deserialization_error(
												"Error reading layer \"%x\": node \"%x\" not found!", 
												layer.unique_name,
												node_id
											);
										}
									}
								}
							}
							else {
								if (strict) {
									throw augs::json_deserialization_error(
										"Error reading layer \"%x\": Node identifier must be a string!", 
										layer.unique_name
									);
								}
							}
						}
					}
				};

				auto allocate_layer = [&]() {
					const editor_layer_id layer_id = loaded.layers.pool.allocate(std::move(layer));
					loaded.layers.order.push_back(layer_id);
				};

				if (create_fallback_node_order) {
					layer.hierarchy.nodes = std::move(fallback_node_order);
					nodes_registered = layer.hierarchy.nodes.size();

					allocate_layer();
					break;
				}
				else {
					read_layer_nodes();
					allocate_layer();
				}
			}

			if (strict) {
				if (nodes_registered < node_name_map.size()) {
					throw augs::json_deserialization_error(
						"Error reading layers:\n%x nodes weren't assigned to any layer!", 
						node_name_map.size() - nodes_registered
					);
				}
			}
		};

		auto create_all_missing_modes = [&]() {
			auto& modes = loaded.get_game_modes();

			editor_game_mode_resource new_game_mode;
			::setup_game_mode_defaults(new_game_mode.editable, officials_map);

			auto register_if_missing = [&]<typename S>(S) {
				auto key = S::get_identifier();
				if (found_in(mode_map, key)) {
					return;
				}

				new_game_mode.type.set<S>();
				new_game_mode.unique_name = key;

				const auto result = modes.allocate(new_game_mode);
				register_new_resource(key, result);
			};

			register_if_missing(editor_quick_test_mode());
			register_if_missing(editor_bomb_defusal_mode());
			register_if_missing(editor_gun_game_mode());
		};

		auto unstringify_resource_id = [&]<typename R>(editor_typed_resource_id<R>& typed_id) {
			if constexpr(std::is_same_v<R, editor_game_mode_resource>) {
				::unpack_string_id(mode_map, typed_id, strict);
			}
			else {
				::unpack_string_id(resource_map, typed_id, strict);
			}
		};

		auto unstringify_resource_ids = [&]() {
			::on_each_resource_id_in_project(loaded, unstringify_resource_id);
		};

		auto unstringify_node_id = [&]<typename N>(editor_typed_node_id<N>& typed_id) {
			::unpack_string_id(node_name_map, typed_id, strict);
		};

		auto unstringify_node_ids = [&]() {
			::on_each_node_id_in_project(loaded, unstringify_node_id);
		};

		initialize_project_structs();
		read_project_structs();

		read_external_resources();
		read_materials();

		read_nodes();
		read_layers();

		read_modes();
		create_all_missing_modes();

		unstringify_resource_ids();
		unstringify_node_ids();

		if (!loaded.playtesting.mode.is_set()) {
			::setup_default_playtesting_mode(loaded.playtesting, loaded.get_game_modes());
		}

		if (!loaded.settings.default_server_mode.is_set()) {
			::setup_default_server_mode(loaded.settings, loaded.get_game_modes());
		}

		if (output_arena_hash != nullptr) {
			*output_arena_hash = augs::secure_hash(loaded_project_json);
		}

		return loaded;
	}

	external_resource_database read_only_external_resources(
		const augs::path_type& project_dir,
		const std::string& loaded_project_json
	) {
		external_resource_database database;

		const auto document = augs::json_document_from(loaded_project_json);
		const auto maybe_externals = FindArray(document, "external_resources");

		if (!maybe_externals) {
			return database;
		}

		std::unordered_set<augs::path_type> existing;

		for (auto& resource : *maybe_externals) {
			if (!resource.IsObject()) {
				continue;
			}

			if (!resource.HasMember("path") || !resource["path"].IsString()) {
				continue;
			}

			if (!resource.HasMember("file_hash") || !resource["file_hash"].IsString()) {
				continue;
			}

			const auto file_hash = resource["file_hash"].GetString();
			const auto untrusted_path = resource["path"].GetString();

			std::visit(
				[&]<typename R>(const R& result) {
					if constexpr(std::is_same_v<R, sanitization::forbidden_path_type>) {
						const auto err = typesafe_sprintf(
							"Failed to load %x:\n%x",
							untrusted_path,
							sanitization::describe(result)
						);

						throw augs::json_deserialization_error(err);
					}
					else if constexpr(std::is_same_v<R, augs::path_type>) {
						const auto& path = result;

						if (found_in(existing, path)) {
							throw augs::json_deserialization_error("Duplicate entries for %x!", path);

							return;
						}

						existing.emplace(path);
						database.emplace_back(path, augs::to_secure_hash_byte_format(file_hash));
					}
					else {
						static_assert(always_false_v<R>, "Non-exhaustive if constexpr");
					}
				},

				sanitization::sanitize_downloaded_file_path(project_dir, untrusted_path)
			);
		}

		return database;
	}

	editor_project_about read_only_project_about(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		return augs::from_json_subobject<editor_project_about>(document, "about");
	}

	editor_project_meta read_only_project_meta(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		return augs::from_json_subobject<editor_project_meta>(document, "meta");
	}

	version_timestamp_string read_only_project_timestamp(const std::string& project_json) {
		const auto document = augs::json_document_from(project_json);

		return augs::from_json_subobject<editor_project_meta>(document, "meta").version_timestamp;
	}
}
