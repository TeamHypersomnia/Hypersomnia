#pragma once
#include <vector>
#include <string>
 
#include "application/setups/debugger/property_editor/browsed_path_entry_base.h"

template <class id_type>
struct pathed_asset_entry : public browsed_path_entry_base<id_type> {
	using base = browsed_path_entry_base<id_type>;

	id_type id;
	std::vector<std::string> using_locations;

	bool missing = false;

	bool used() const {
		return using_locations.size() > 0;
	}

	pathed_asset_entry() = default;
	pathed_asset_entry(
		const maybe_official_path<id_type>& from,
	   	const id_type id
	) :
		base(from),
		id(id)
	{}
};

