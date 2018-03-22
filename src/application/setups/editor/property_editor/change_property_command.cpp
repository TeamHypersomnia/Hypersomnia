#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/change_entity_property_command.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/commands/change_common_state_command.h"
#include "application/setups/editor/property_editor/change_property_command.h"

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
	auto& cosm = in.folder.work->world;

	common.reset_timestamp();

	ensure(value_after_change.empty());

	self.access_each_property(
		cosm,
		[&](auto& field) {
			augs::from_bytes(new_value, field);
			return callback_result::CONTINUE;
		}
	);
}

using namespace augs;

template <class M, class T>
static void detail_write_bytes(M& to, const T& from, const std::size_t bytes_count) {
	if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		const std::byte* location = reinterpret_cast<const std::byte*>(std::addressof(from));
		to.write(location, bytes_count);
	}
	else {
		augs::write_bytes(to, from);
	}
}

template <class M, class T>
static void detail_read_bytes(M& from, T& to, const std::size_t bytes_count) {
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
	auto& cosm = in.folder.work->world;
	auto& self = *static_cast<D*>(this);

	ensure(value_before_change.empty());

	const auto trivial_element_size = value_after_change.size();

	auto before_change_data = ref_memory_stream(value_before_change);
	auto after_change_data = cref_memory_stream(value_after_change);

	self.access_each_property(
		cosm,
		[&](auto& field) {
			detail_write_bytes(before_change_data, field, trivial_element_size);

			detail_read_bytes(after_change_data, field, trivial_element_size);
			after_change_data.set_read_pos(0u);
			return callback_result::CONTINUE;
		}	
	);

	value_after_change.clear();
}

template <class D>
void change_property_command<D>::undo(const editor_command_input in) {
	auto& cosm = in.folder.work->world;
	auto& self = *static_cast<D*>(this);

	bool read_once = true;

	ensure(value_after_change.empty());

	const auto trivial_element_size = value_before_change.size() / self.count_affected();

	auto before_change_data = cref_memory_stream(value_before_change);
	auto after_change_data = ref_memory_stream(value_after_change);

	self.access_each_property(
		cosm,
		[&](auto& field) {
			if (read_once) {
				detail_write_bytes(after_change_data, field, trivial_element_size); 
				read_once = false;
			}

			detail_read_bytes(before_change_data, field, trivial_element_size);
			return callback_result::CONTINUE;
		}	
	);

	value_before_change.clear();
}

template class change_property_command<change_flavour_property_command>;
template class change_property_command<change_entity_property_command>;
template class change_property_command<change_common_state_command>;
