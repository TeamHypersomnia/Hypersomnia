#include "application/setups/editor/project/editor_project_readwrite.h"
#include "3rdparty/rapidjson/include/rapidjson/rapidjson.h"

#include "augs/readwrite/json_readwrite.h"

namespace editor_project_readwrite {
	editor_project read_project_json(const augs::path_type& json_path) {
		return augs::from_json_file<editor_project>(json_path);
	}

	editor_project_about read_only_project_about(const augs::path_type& json_path) {
		/* TDOO: use lookahead parser */
		return read_project_json(json_path).about;
	}
}
