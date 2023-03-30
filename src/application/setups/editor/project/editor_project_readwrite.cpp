#include <unordered_set>
#include "augs/string/path_sanitization.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/detail/is_editor_typed_resource.h"
#include "application/setups/editor/resources/resource_traits.h"

namespace augs {
	template <class T, class R>
	void to_json_value(T& out, const editor_typed_resource_id<R>& from) {
		out.String(from._serialized_resource_name);
	}

	template <class T, class R>
	void from_json_value(T& from, editor_typed_resource_id<R>& out) {
		if (from.IsString()) {
			out._serialized_resource_name = from.GetString();
		}
	}
}

#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/project/editor_project.hpp"
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

#if 0
template <class R>
void unpack_string_id(resource_name_to_id& ids, editor_typed_resource_id<R>& id) {
	if (auto found = mapped_or_nullptr(ids.get_for<R>(), id._serialized_resource_name)) {
		id = *found;
	}
}
#else
template <class R>
void unpack_string_id(resource_name_to_id& ids, editor_typed_resource_id<R>& id) {
	if (auto found = mapped_or_nullptr(ids, id._serialized_resource_name)) {
		if ((*found).type_id.template is<R>()) {
			id = editor_typed_resource_id<R>::from_generic(*found);
		}
	}
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

namespace editor_project_readwrite {
	void write_editor_view(const augs::path_type& json_path, const editor_view& view) {
		augs::save_as_json(view, json_path);
	}

	editor_view read_editor_view(const augs::path_type& json_path) {
		return augs::from_json_file<editor_view>(json_path);
	}

	void write_project_json(const augs::path_type& json_path, const editor_project& project) {
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		(void)project;

		/*
			We'll have to pass proper id handlers here.
			Global settings may specify some resources.
		*/

		//augs::write_json(writer, project);

		/*
			Layer hierarchies and nodes are ignored when writing.
			We have to write them manually.
		*/

		augs::save_as_text(json_path, s.GetString());
	}

	template <class T>
	struct recurse_to_find_ids {
		static constexpr bool value = !augs::json_ignore_v<T> && !augs::has_custom_to_json_value_v<T>;
	};

	template <class P, class F>
	void on_each_resource_id_in_project(P& project, F callback) {
		auto handle = [&](auto& object) {
			using Field = remove_cref<decltype(object)>;

			if constexpr(is_editor_typed_resource_id_v<Field>) {
				callback(object);
			}
		};

		auto traverse_object = [&](auto& object) {
			augs::on_each_object_in_object<recurse_to_find_ids>(object, handle);
		};

		traverse_object(project);

		project.resources.pools.for_each(
			[&](auto& resource) {
				traverse_object(resource.editable);
			}
		);

		project.nodes.pools.for_each(
			[&](auto& node) {
				callback(node.resource_id);
				traverse_object(node.editable);
			}
		);

		for (auto& layer : project.layers.pool) {
			traverse_object(layer.editable);
		}
	}

	editor_project read_project_json(
		const augs::path_type& json_path,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map,
		const bool strict
	) {
		(void)officials;

		auto resource_map = officials_map.create_name_to_id_map();

		auto register_new_resource = [&](const auto name_id, auto& allocation_result) {
			auto& allocated = allocation_result.object;

			using O = remove_cref<decltype(allocated)>;
			using Id = editor_typed_resource_id<O>;

			const auto typed_id = Id::from_raw(allocation_result.key, false);

			const auto result = resource_map.try_emplace(name_id, typed_id.operator editor_resource_id());

			if (strict) {
				if (!result.second) {
					throw augs::json_deserialization_error("Duplicate resource: \"%x\"", name_id);
				}
			}
		};

		editor_project loaded;

		auto initialize_project_structs = [&]() {
			::setup_project_defaults(loaded.settings, loaded.get_game_modes(), officials_map);

			/* 
				This doesn't set the playtesting mode (done later below) 
				but maybe there will be other important properties in the future
			*/

			::setup_project_defaults(loaded.playtesting, loaded.get_game_modes(), officials_map);
		};

		const auto project_dir = json_path.parent_path();
		const auto document = augs::json_document_from(json_path);

		auto read_project_structs = [&]() {

			/*
				Will leave out non-trivial fields like layers, nodes and resources.
				(see: static constexpr bool json_ignore = true)
				Only stuff like about, meta, settings etc. get loaded in this call.
			*/

			static_assert(augs::has_custom_to_json_value_v<decltype(editor_playtesting_settings::mode)>);

			augs::read_json(document, loaded);
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

				auto read_into = [&](auto& specific_game_mode) {
					augs::read_json(mode.value, specific_game_mode);
				};

				if (key == "playtesting") {
					new_game_mode.type.set<editor_playtesting_mode>();
					read_into(new_game_mode.editable.playtesting);
				}
				else if (key == "bomb_defusal") {
					new_game_mode.type.set<editor_bomb_defusal_mode>();
					read_into(new_game_mode.editable.bomb_defusal);
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

				if (!resource.HasMember("id") || !resource["id"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"id\" property for %x!", resource["path"].GetString());
					}

					continue;
				}

				if (!resource.HasMember("file_hash") || !resource["file_hash"].IsString()) {
					if (strict) {
						throw augs::json_deserialization_error("Missing \"file_hash\" property for %x!", resource["path"].GetString());
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
						::setup_resource_defaults(typed_resource, officials_map);
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

		auto read_internal_resources = [&]() {
			/* 
				For now we won't have any.
				All of them are official.
			*/
		};

		struct node_meta {
			editor_node_id id = editor_node_id();
			bool assigned_to_layer = false;
		};

		std::unordered_map<std::string, node_meta> node_map;

		auto read_nodes = [&]() {
			const auto maybe_nodes = FindArray(document, "nodes");

			if (maybe_nodes != std::nullopt) {
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
							throw augs::json_deserialization_error("Invalid \"type\" property for node \"%x\".\nResource not found!", id);
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

							const auto map_result = node_map.try_emplace(id, node_meta());

							if (const bool is_unique = map_result.second) {
								auto& pool = loaded.nodes.pools.get_for<node_type>();

								const auto [new_raw_id, new_node] = pool.allocate();

								new_node.resource_id = typed_resource_id;
								new_node.unique_name = id;

								::setup_node_defaults(new_node, typed_resource);
								augs::read_json(json_node, new_node.editable);

								const auto new_typed_id = editor_typed_node_id<node_type>::from_raw(new_raw_id);
								(*map_result.first).second.id = new_typed_id.operator editor_node_id();
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

				editor_layer layer;
				layer.unique_name = *id;

				augs::read_json(json_layer, layer.editable);

				auto read_layer_nodes = [&]() {
					if (const auto maybe_nodes = FindArray(json_layer, "nodes")) {
						for (auto& layer_node : *maybe_nodes) {
							if (layer_node.IsString()) {
								const auto node_id = layer_node.GetString();
								
								if (const auto found_node = mapped_or_nullptr(node_map, node_id)) {
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
									if (strict) {
										throw augs::json_deserialization_error(
											"Error reading layer \"%x\": node \"%x\" not found!", 
											layer.unique_name,
											node_id
										);
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

				read_layer_nodes();

				const editor_layer_id layer_id = loaded.layers.pool.allocate(std::move(layer));
				loaded.layers.order.push_back(layer_id);
			}

			if (strict) {
				if (nodes_registered < node_map.size()) {
					throw augs::json_deserialization_error(
						"Error reading layers:\n%x nodes weren't assigned to any layer!", 
						node_map.size() - nodes_registered
					);
				}
			}
		};

		auto create_fallback_playtesting_mode_if_none = [&]() {
			auto& modes = loaded.get_game_modes();

			if (modes.empty()) {
				editor_game_mode_resource new_game_mode;
				::setup_game_mode_defaults(new_game_mode.editable, officials_map);

				const auto key = "playtesting";
				new_game_mode.type.set<editor_playtesting_mode>();
				new_game_mode.unique_name = key;

				const auto result = modes.allocate(std::move(new_game_mode));
				register_new_resource(key, result);
			}
		};

		auto unstringify_resource_ids = [&]() {
			auto resolve = [&](auto& typed_id) {
				::unpack_string_id(resource_map, typed_id);
			};

			on_each_resource_id_in_project(loaded, resolve);
		};

		initialize_project_structs();
		read_project_structs();

		read_modes();
		create_fallback_playtesting_mode_if_none();

		read_external_resources();
		read_internal_resources();

		read_nodes();
		read_layers();

		unstringify_resource_ids();

		{
		}

		if (!loaded.playtesting.mode.is_set()) {
			::setup_default_playtesting_mode(loaded.playtesting, loaded.get_game_modes());
		}

		if (!loaded.settings.default_server_mode.is_set()) {
			::setup_default_server_mode(loaded.settings, loaded.get_game_modes());
		}

		return loaded;
	}

	editor_project_about read_only_project_about(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		return augs::from_json_subobject<editor_project_about>(document, "about");
	}

	editor_project_meta read_only_project_meta(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		return augs::from_json_subobject<editor_project_meta>(document, "meta");
	}
}
