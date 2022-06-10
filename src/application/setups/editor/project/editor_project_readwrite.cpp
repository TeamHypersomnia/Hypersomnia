#include "application/setups/editor/project/editor_project_readwrite.h"
#include "augs/readwrite/json_readwrite.h"

namespace editor_project_readwrite {
	editor_project read_project_json(const augs::path_type& json_path) {
		return augs::from_json_file<editor_project>(json_path);
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
