#pragma once
#include <vector>
#include "augs/filesystem/path.h"

namespace sol {
	class state;
}

struct editor_recent_paths {
	// GEN INTROSPECTOR struct editor_recent_paths
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR

	editor_recent_paths(sol::state& lua);
	void add(sol::state& lua, augs::path_type path);
	void clear(sol::state&);
	bool empty() const;
};