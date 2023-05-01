#include "augs/enums/callback_result.h"

#include "game/cosmos/typed_entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "application/intercosm.h"
#include "application/setups/debugger/debugger_folder.h"

#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/commands/change_entity_property_command.h"
#include "application/setups/debugger/commands/change_flavour_property_command.h"
#include "application/setups/debugger/commands/change_common_state_command.h"
#include "application/setups/debugger/commands/change_grouping_command.h"
#include "application/setups/debugger/commands/change_property_command.h"
#include "application/setups/debugger/commands/asset_commands.h"

#include "application/setups/debugger/commands/detail/debugger_property_accessors.h"
#include "augs/misc/from_concave_polygon.h"

#include "augs/readwrite/to_bytes.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/debugger/commands/debugger_command_sanitizer.h"

template <class D>
std::string change_property_command<D>::describe() const {
	return built_description;
}

template <class D>
void change_property_command<D>::refresh_other_state(const debugger_command_input in) {
	(void)in;

	if constexpr(std::is_same_v<D, change_asset_property_command<assets::image_id>>) {
		auto& self = *static_cast<D*>(this);

		for (const auto& i : self.affected_assets) {
			in.folder.commanded->work.update_offsets_of(i, changer_callback_result::DONT_REFRESH);
		}

		if (self.property_id.field.type_id.template is<image_shape_type>()) {
			auto& cosm = in.folder.commanded->work.world;

			cosm.change_common_significant(
				[&](cosmos_common_significant& common) {
					for (const auto& i : self.affected_assets) {
						auto& sh = common.logical_assets.get_offsets(i).non_standard_shape;
						augs::refresh_convex_partitioning(sh);

						in.folder.commanded->work.viewables.image_definitions[i].meta.offsets.non_standard_shape.convex_partition = sh.convex_partition;
					};

					return changer_callback_result::DONT_REFRESH;
				}
			);
		}

		if (!self.affected_assets.empty()) {
			auto reinfer = [&]() {
				in.folder.commanded->work.update_offsets_of(self.affected_assets.front(), changer_callback_result::REFRESH);
			};

			reinfer();
		}	
	}
}

template <class D>
void change_property_command<D>::rewrite_change_internal(
	const debugger_command_input in
) {
	auto& self = *static_cast<D*>(this);

	common.reset_timestamp();

	debugger_property_accessors::access_each_property(
		self,
		in,
		[&](auto& field) {
			augs::from_bytes(value_after_change, field);
			return callback_result::CONTINUE;
		}
	);

	refresh_other_state(in);
}

template <class Archive, class T>
static void write_object_or_trivial_marker(Archive& ar, const T& from, const std::size_t bytes_count) {
	if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		const std::byte* location = reinterpret_cast<const std::byte*>(std::addressof(from));
		ar.write(location, bytes_count);
	}
	else {
		(void)bytes_count;
		augs::write_bytes(ar, from);
	}
}

template <class Archive, class T>
static void read_object_or_trivial_marker(Archive& ar, T& to, const std::size_t bytes_count) {
	if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		std::byte* location = reinterpret_cast<std::byte*>(std::addressof(to));
		ar.read(location, bytes_count);
	}
	else {
		(void)bytes_count;
		augs::read_bytes(ar, to);
	}
}

template <class D>
void change_property_command<D>::redo(const debugger_command_input in) {
	auto& self = *static_cast<D*>(this);

	self.clear_undo_state();

	const auto trivial_element_size = value_after_change.size();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	auto after_change_data = augs::cref_memory_stream(value_after_change);

	debugger_property_accessors::access_each_property(
		self,
		in,
		[&](auto& field) {
			write_object_or_trivial_marker(before_change_data, field, trivial_element_size);

			read_object_or_trivial_marker(after_change_data, field, trivial_element_size);
			after_change_data.set_read_pos(0u);
			return callback_result::CONTINUE;
		}	
	);

	refresh_other_state(in);
}

template <class D>
void change_property_command<D>::undo(const debugger_command_input in) {
	auto& self = *static_cast<D*>(this);

	const auto trivial_element_size = values_before_change.size() / self.count_affected();

	auto before_change_data = augs::cref_memory_stream(values_before_change);

	debugger_property_accessors::access_each_property(
		self,
		in,
		[&](auto& field) {
			read_object_or_trivial_marker(before_change_data, field, trivial_element_size);
			return callback_result::CONTINUE;
		}	
	);

	refresh_other_state(in);
	self.clear_undo_state();
}

template <class D>
void change_property_command<D>::clear_undo_state() {
	values_before_change.clear();
}

template class change_property_command<change_flavour_property_command>;
template class change_property_command<change_initial_component_property_command>;
template class change_property_command<change_entity_property_command>;

template class change_property_command<change_common_state_command>;
template class change_property_command<change_group_property_command>;
template class change_property_command<change_current_mode_property_command>;
template class change_property_command<change_mode_player_property_command>;
template class change_property_command<change_ruleset_property_command>;
template class change_property_command<change_rulesets_meta_property>;

template class change_property_command<change_asset_property_command<assets::image_id>>;
template class change_property_command<change_asset_property_command<assets::sound_id>>;

template class change_property_command<change_asset_property_command<assets::plain_animation_id>>;
template class change_property_command<change_asset_property_command<assets::particle_effect_id>>;

void change_entity_property_command::sanitize(const debugger_command_input in) {
	sanitize_affected_entities(in, affected_entities, [this](const auto& entry) {
		entity_id target;
		target.type_id = type_id;
		target.raw = entry;
		return target;
	});
}

static_assert(has_member_sanitize_v<change_entity_property_command, debugger_command_input>);
