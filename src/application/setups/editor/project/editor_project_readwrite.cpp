#include "application/setups/editor/project/editor_project_readwrite.h"
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
	void write_project_json(const augs::path_type& json_path, const editor_project& project) {
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		augs::write_json(writer, project);

		/*
			Layer hierarchies and nodes are ignored when writing.
			We have to write them manually.
		*/

		augs::save_as_text(json_path, s.GetString());
	}

	editor_project read_project_json(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		editor_project loaded;
		augs::read_json(document, loaded);

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
