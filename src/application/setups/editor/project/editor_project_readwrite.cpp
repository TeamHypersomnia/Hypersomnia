#include "application/setups/editor/project/editor_project_readwrite.h"
#include "augs/readwrite/json_readwrite.h"

template <class T, class F>
std::optional<T> GetIf(F& from, const std::string& label) {
	if (from.HasMember(label) && from[label].template Is<T>()) {
		return from[label].template Get<T>();
	}

	return std::nullopt;
}

namespace editor_project_readwrite {
	editor_project read_project_json(const augs::path_type& json_path) {
		const auto document = augs::json_document_from(json_path);

		editor_project loaded;
		augs::read_json(document, loaded);

		/*
			Layer hierarchies and nodes are ignored when reading.
			We have to load them manually.
		*/

		{
			const auto name_to_layer = loaded.make_name_to_layer_map();

			if (document.HasMember("nodes") && document["nodes"].IsArray()) {
				for (auto& node : document["nodes"].GetArray()) {
					if (!node.IsObject()) {
						continue;
					}

					/* First we determine the type of this object */
					const auto new_id = editor_node_id();

					auto name = GetIf<std::string>(node, "name");
					auto layer = GetIf<std::string>(node, "layer");

					if (layer == std::nullopt) {
						continue;
					}

					if (name != std::nullopt) {
						loaded.nodes.node_names[new_id] = *name;
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
