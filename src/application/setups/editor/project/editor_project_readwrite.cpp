#include "application/setups/editor/editor_view.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/project/editor_project_readwrite.h"

#include "application/setups/editor/editor_official_resource_map.h"
#include "application/setups/editor/defaults/editor_game_mode_defaults.h"
#include "application/setups/editor/defaults/editor_project_defaults.h"

#include "augs/misc/pool/pool_allocate.h"
#include "augs/readwrite/json_readwrite.h"

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

	editor_project read_project_json(
		const augs::path_type& json_path,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map
	) {
		(void)officials;

		const auto document = augs::json_document_from(json_path);

		editor_project loaded;
		::setup_project_defaults(loaded.settings, loaded.get_game_modes(), officials_map);

		/* 
			This doesn't set the playtesting mode (done later below) 
			but maybe there will be other important properties in the future
		*/

		::setup_project_defaults(loaded.playtesting, loaded.get_game_modes(), officials_map);

		loaded.meta = read_only_project_meta(json_path);
		loaded.about = read_only_project_about(json_path);

		{
			if (const auto maybe_modes = FindObject(document, "game_modes")) {
				auto& modes = loaded.get_game_modes();

				for (auto& mode : *maybe_modes) {
					if (!mode.value.IsObject()) {
						continue;
					}

					const auto name = std::string(mode.name.GetString());

					editor_game_mode_resource new_game_mode;
					::setup_game_mode_defaults(new_game_mode.editable, officials_map);

					auto read_into = [&](auto& typed) {
						(void)typed;
					};

					if (name == "playtesting") {
						new_game_mode.type.set<editor_playtesting_mode>();
						read_into(new_game_mode.editable.playtesting);
					}
					else if (name == "bomb_defusal") {
						new_game_mode.type.set<editor_bomb_defusal_mode>();
						read_into(new_game_mode.editable.bomb_defusal);
					}

					new_game_mode.unique_name = name;

					modes.allocate(std::move(new_game_mode));
				}
			}
		}

		auto create_fallback_playtesting_mode_if_none = [&]() {
			auto& modes = loaded.get_game_modes();

			if (modes.empty()) {
				editor_game_mode_resource new_game_mode;
				::setup_game_mode_defaults(new_game_mode.editable, officials_map);

				new_game_mode.type.set<editor_playtesting_mode>();
				new_game_mode.unique_name = "playtesting";

				modes.allocate(std::move(new_game_mode));
			}
		};

		create_fallback_playtesting_mode_if_none();

		/*
			Layer hierarchies and nodes are ignored when reading.
			We have to load them manually.
		*/

		{
			const auto name_to_layer = loaded.layers.make_name_to_layer_map();
			const auto maybe_nodes = FindArray(document, "nodes");
			const auto maybe_prefabs = FindObject(document, "prefabs");

			if (maybe_prefabs != std::nullopt) {

			}

			if (maybe_nodes != std::nullopt) {
				for (auto& node : *maybe_nodes) {
					if (!node.IsObject()) {
						continue;
					}

					auto maybe_type = GetIf<std::string>(node, "type");

					if (maybe_type == std::nullopt) {
						continue;
					}

					auto& type = maybe_type.value();

					bool is_official = false;
					(void)is_official;

					if (type.front() == '@') {
						type.erase(type.begin());

						is_official = true;
					}

					const bool is_prefab   = type.front() == '[' && type.back() == ']';
					const bool is_internal = type.front() == '<' && type.back() == '>';

					if (is_prefab) {

					}
					else if (is_internal) {

					}
					else {

					}

					/* 
						First we determine the type of this object
					*/

					const auto new_id = editor_node_id();

					auto name = GetIf<std::string>(node, "name");
					auto layer = GetIf<std::string>(node, "layer");

					if (layer == std::nullopt) {
						continue;
					}

					if (name != std::nullopt) {
						loaded.nodes.names[new_id] = *name;
					}
				}
			}
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
