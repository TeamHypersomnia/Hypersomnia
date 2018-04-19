#include "augs/enums/callback_result.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/change_entity_property_command.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/commands/change_common_state_command.h"
#include "application/setups/editor/commands/change_grouping_command.h"
#include "application/setups/editor/commands/change_property_command.h"
#include "application/setups/editor/commands/asset_commands.h"

#include "augs/readwrite/byte_readwrite.h"

template <class D>
std::string change_property_command<D>::describe() const {
	return built_description;
}

template <class D>
void change_property_command<D>::rewrite_change(
	const std::vector<std::byte>& new_value,
	const editor_command_input in
) {
	auto& self = *static_cast<D*>(this);

	common.reset_timestamp();

	/* 
		At this point, the command can only be undone or rewritten,  
		so it makes sense that the storage for value after change is empty.
	*/

	ensure(value_after_change.empty());

	self.access_each_property(
		in,
		[&](auto& field) {
			augs::from_bytes(new_value, field);
			return callback_result::CONTINUE;
		}
	);
}

template <class M, class T>
static void write_object_or_trivial_marker(M& to, const T& from, const std::size_t bytes_count) {
	if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		const std::byte* location = reinterpret_cast<const std::byte*>(std::addressof(from));
		to.write(location, bytes_count);
	}
	else {
		augs::write_bytes(to, from);
	}
}

template <class M, class T>
static void read_object_or_trivial_marker(M& from, T& to, const std::size_t bytes_count) {
	if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		std::byte* location = reinterpret_cast<std::byte*>(std::addressof(to));
		from.read(location, bytes_count);
	}
	else {
		augs::read_bytes(from, to);
	}
}

template <class D>
void change_property_command<D>::redo(const editor_command_input in) {
	auto& self = *static_cast<D*>(this);

	ensure(values_before_change.empty());

	const auto trivial_element_size = value_after_change.size();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	auto after_change_data = augs::cref_memory_stream(value_after_change);

	self.access_each_property(
		in,
		[&](auto& field) {
			write_object_or_trivial_marker(before_change_data, field, trivial_element_size);

			read_object_or_trivial_marker(after_change_data, field, trivial_element_size);
			after_change_data.set_read_pos(0u);
			return callback_result::CONTINUE;
		}	
	);

	value_after_change.clear();
}

template <class D>
void change_property_command<D>::undo(const editor_command_input in) {
	auto& self = *static_cast<D*>(this);

	bool read_once = true;

	ensure(value_after_change.empty());

	const auto trivial_element_size = values_before_change.size() / self.count_affected();

	auto before_change_data = augs::cref_memory_stream(values_before_change);
	auto after_change_data = augs::ref_memory_stream(value_after_change);

	self.access_each_property(
		in,
		[&](auto& field) {
			if (read_once) {
				write_object_or_trivial_marker(after_change_data, field, trivial_element_size); 
				read_once = false;
			}

			read_object_or_trivial_marker(before_change_data, field, trivial_element_size);
			return callback_result::CONTINUE;
		}	
	);

	values_before_change.clear();
}

template class change_property_command<change_flavour_property_command>;
template class change_property_command<change_entity_property_command>;
template class change_property_command<change_common_state_command>;
template class change_property_command<change_group_property_command>;

template class change_property_command<change_asset_property_command<assets::image_id, false>>;
template class change_property_command<change_asset_property_command<assets::image_id, true>>;
