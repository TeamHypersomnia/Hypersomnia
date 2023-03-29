#pragma once
#include <string_view>
#include "augs/filesystem/path.h"
#include "augs/filesystem/file_time_type.h"

struct editor_pathed_resource {
	static constexpr bool json_ignore = true;

	// GEN INTROSPECTOR struct editor_pathed_resource
	augs::path_type path_in_project;
	std::string file_hash;
	augs::file_time_type stamp_when_hashed;
	// END GEN INTROSPECTOR

	editor_pathed_resource(
		const augs::path_type& path_in_project, 
		const std::string& file_hash,
		const augs::file_time_type& stamp_when_hashed
	);

	void set_hash_stamp(const augs::file_time_type& stamp_when_hashed);

	void maybe_rehash(const augs::path_type& full_path, const augs::file_time_type& fresh_stamp);

	std::string get_display_name() const;

	augs::path_type path_in(const augs::path_type& folder) const {
		return folder / path_in_project;
	}
};
