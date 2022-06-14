#pragma once
#include "augs/filesystem/path.h"
#include "application/setups/editor/project/editor_project.h"

namespace editor_project_readwrite {
	void write_project_json(const augs::path_type& json_path, const editor_project&);

	editor_project read_project_json(const augs::path_type& json_path);

	editor_project_about read_only_project_about(const augs::path_type& json_path);
	editor_project_meta read_only_project_meta(const augs::path_type& json_path);

	void write_project_json(const augs::path_type& json_path, const editor_project&);
}

