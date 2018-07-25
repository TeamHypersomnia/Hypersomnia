#pragma once

struct flavour_property_id;

struct create_flavour_command;
struct delete_flavour_command;

template <class id_type>
struct asset_property_id;

template <class id_type>
struct forget_asset_id_command;

template <class id_type>
struct create_unpathed_asset_id_command;

template <class id_type>
struct duplicate_asset_command;

class cosmos_common_significant_access {
	friend flavour_property_id;

	/* Some classes for editor must be privileged */
	friend create_flavour_command;
	friend delete_flavour_command;

	template <class id_type>
	friend struct asset_property_id;

	template <class id_type>
	friend struct forget_asset_id_command;

	template <class id_type>
	friend struct create_unpathed_asset_id_command;

	template <class id_type>
	friend struct duplicate_asset_command;

	cosmos_common_significant_access() {}
};
