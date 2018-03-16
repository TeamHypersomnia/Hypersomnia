#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/property_editor/change_property_command.h"

#include "augs/readwrite/byte_readwrite.h"

template <class D>
std::string change_property_command<D>::describe() const {
	return built_description;
}

template <class D>
void change_property_command<D>::rewrite_change(
	std::vector<std::byte>&& new_value,
	const editor_command_input in
) {
	auto& self = *static_cast<D*>(this);
	auto& cosm = in.folder.work->world;

	common.timestamp = {};

	self.access_property(
		cosm,
		[&](auto& field, const auto& reinferrable_object) {
			augs::from_bytes(std::move(new_value), field);
			return maybe_reinfer(reinferrable_object);
		}
	);
}

template <class D>
void change_property_command<D>::redo(const editor_command_input in) {
	auto& cosm = in.folder.work->world;
	auto& self = *static_cast<D*>(this);

	self.access_property(
		cosm,
		[&](auto& field, const auto& reinferrable_object) {
			const auto bytes_count = value_after_change.size();
			value_before_change = detail_field_to_bytes(field, bytes_count);

			augs::from_bytes(std::move(value_after_change), field);
			return maybe_reinfer(reinferrable_object);
		}	
	);
}

template <class D>
void change_property_command<D>::undo(const editor_command_input in) {
	auto& cosm = in.folder.work->world;
	auto& self = *static_cast<D*>(this);

	self.access_property(
		cosm,
		[&](auto& field, const auto& reinferrable_object) {
			const auto bytes_count = value_before_change.size();
			value_after_change = detail_field_to_bytes(field, bytes_count);

			augs::from_bytes(std::move(value_before_change), field);
			return maybe_reinfer(reinferrable_object);
		}	
	);
}

template class change_property_command<change_flavour_property_command>;
