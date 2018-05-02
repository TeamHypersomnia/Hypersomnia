#pragma once
#include "view/asset_location_context.h"

template <class T>
class asset_definition_view {
protected:
	const T& def;
	const augs::path_type resolved_source_path;

public:
	asset_definition_view(
		const asset_location_context& project_dir,
		const T& d
	) : 
		def(d), 
		resolved_source_path(d.get_source_path().resolve(project_dir))
	{
	}

	const auto& get_resolved_source_path() const {
		return resolved_source_path;
	}
};
