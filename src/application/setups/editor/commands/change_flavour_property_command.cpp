#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"

#include "application/setups/editor/gui/editor_properties_gui.h"
#include "augs/readwrite/byte_readwrite.h"

template <class C, class F>
void on_property(
	const flavour_property_id property_id,
   	C& cosm,
   	F callback
) {
	cosm.change_common_significant([&](cosmos_common_significant& common_signi) {
		auto result = changer_callback_result::DONT_REFRESH;

		common_signi.on_flavour(
			property_id.flavour_id,
			[&](auto& flavour) {
				get_by_dynamic_index(
					flavour.invariants,
					property_id.invariant_id,
					[&](auto& invariant) {
						get_by_dynamic_id(
							edited_field_type_id::list_type(),
							property_id.field_type,
							[&](const auto& t) {
								using T = std::decay_t<decltype(t)>;

								const auto invariant_location = reinterpret_cast<std::byte*>(std::addressof(invariant));
								const auto location = reinterpret_cast<T*>(invariant_location + property_id.field_offset);

								result = callback(*location, invariant);
							}
						);
					}
				);
			}
		);

		return result;
	});
}


template <class T>
static constexpr bool should_reinfer(const T& invariant) {
	return should_reinfer_when_tweaking_v<T>;
}

template <class T>
static auto maybe_reinfer(const T& invariant) {
	return should_reinfer(invariant) ? changer_callback_result::REFRESH : changer_callback_result::DONT_REFRESH;
}

std::string change_flavour_property_command::describe() const {
	return built_description;
}

void change_flavour_property_command::rewrite_change(
	std::vector<std::byte>&& new_value,
	const editor_command_input in
) {
	auto& cosm = in.folder.work->world;

	common.timestamp = {};

	on_property(
		property_id,
		cosm,
		[&](auto& field, const auto& invariant) {
			augs::from_bytes(std::move(new_value), field);
			return maybe_reinfer(invariant);
		}
	);
}

void change_flavour_property_command::redo(const editor_command_input in) {
	auto& cosm = in.folder.work->world;

	on_property(
		property_id,
		cosm,
		[&](auto& field, const auto& invariant) {
			const auto bytes_count = value_after_change.size();
			value_before_change = detail_field_to_bytes(field, bytes_count);

			augs::from_bytes(std::move(value_after_change), field);
			return maybe_reinfer(invariant);
		}	
	);
}

void change_flavour_property_command::undo(const editor_command_input in) {
	auto& cosm = in.folder.work->world;

	on_property(
		property_id,
		cosm,
		[&](auto& field, const auto& invariant) {
			const auto bytes_count = value_before_change.size();
			value_after_change = detail_field_to_bytes(field, bytes_count);

			augs::from_bytes(std::move(value_before_change), field);
			return maybe_reinfer(invariant);
		}	
	);
}
