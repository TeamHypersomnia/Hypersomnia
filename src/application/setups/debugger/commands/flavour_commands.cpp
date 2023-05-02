#include "augs/string/string_templates.h"
#include "application/setups/debugger/commands/flavour_commands.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/intercosm.h"
#include "application/setups/debugger/commands/asset_commands.h"
#include "augs/readwrite/byte_readwrite.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"

#include "application/setups/debugger/property_debugger/widgets/asset_sane_default_provider.h"
#include "application/setups/debugger/property_debugger/widgets/pathed_asset_widget.h"
#include "application/setups/debugger/detail/find_free_name.h"
#include "augs/misc/pool/pool_allocate.h"

std::string create_flavour_command::describe() const {
	return built_description;
}

void create_flavour_command::redo(const debugger_command_input in) {
	redo_and_copy(in, {});
}

void create_flavour_command::redo_and_copy(const debugger_command_input in, const raw_entity_flavour_id source) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);
			using flavour_type = entity_flavour<E>;

			static const auto entity_type_label = str_ops(format_field_name(get_type_name<E>())).replace_all(" ", "").subject;

			auto& work = in.folder.commanded->work;
			auto& cosm = work.world;
			auto& defs = work.viewables;

			const auto source_flavour = cosm.find_flavour(typed_entity_flavour_id<E>(source));

			const auto basic_name = [source_flavour]() {
				if (source_flavour) {
					return source_flavour->get_name();
				}

				return entity_type_label;
			}();

			auto& flavours = cosm.get_flavours<E>({});
			auto& new_object = base::redo(flavours);

			if (source_flavour != nullptr) {
				new_object = *source_flavour;
			}
			else {
				if constexpr(flavour_type::template has<invariants::sprite>()) {
					const auto provider = asset_sane_default_provider { defs };
					new_object.set(provider.construct<invariants::sprite>());
				}
			}

			const auto suffix = source_flavour != nullptr ? "-Dup-" : "-New-";
			const auto new_name = ::find_free_name(flavours, basic_name + suffix);
			new_object.template get<invariants::text_details>().name = new_name;

			if (source_flavour) {
				built_description = typesafe_sprintf("Cloned flavour: %x", source_flavour->get_name());
			}
			else {
				built_description = typesafe_sprintf("Created flavour: %x", new_name);
			}
		}
	);
}

void create_flavour_command::undo(const debugger_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = in.folder.commanded->work;
			auto& cosm = work.world;
			auto& flavours = cosm.get_flavours<E>({});

			base::undo(flavours);
		}
	);
}

void duplicate_flavour_command::redo(const debugger_command_input in) {
	base::redo_and_copy(in, duplicate_from);
}

void delete_flavour_command::redo(const debugger_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = in.folder.commanded->work;
			auto& cosm = work.world;

			auto& flavours = cosm.get_flavours<E>({});
			
			built_description = "Deleted flavour: " + flavours.get(freed_id).get_name();

			base::redo(flavours);
		}
	);
}

void delete_flavour_command::undo(const debugger_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = in.folder.commanded->work;
			auto& cosm = work.world;

			auto& flavours = cosm.get_flavours<E>({});
			
			base::undo(flavours);
		}
	);
}

std::string delete_flavour_command::describe() const {
	return built_description;
}

std::string instantiate_flavour_command::describe() const {
	return built_description;
}

void instantiate_flavour_command::redo(const debugger_command_input in) {
	auto access = allocate_new_entity_access();
	ensure(!created_id.is_set());

	instantiated_id.type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			in.purge_selections();
			in.interrupt_tweakers();

			const auto flavour_id = typed_entity_flavour_id<E>(instantiated_id.raw);

			auto& work = in.folder.commanded->work;
			auto& cosm = work.world;

			const auto& flavour = cosm.get_flavour(flavour_id);

			built_description = typesafe_sprintf("Instantiated flavour: %x", flavour.get_name());

			auto& selections = in.folder.commanded->view_ids.selected_entities;

			try {
				const auto created_entity = cosmic::specific_create_entity(
					access,
					cosm,
					flavour_id,
					[this](const auto typed_handle, auto&&...) {
						typed_handle.set_logic_transform(transformr(where));
					}
				); 

				created_id = created_entity.get_id();

				selections = { created_id };
			}
			catch (const entity_creation_error&) {
				selections = {};
			}
		}
	);
}

void instantiate_flavour_command::undo(const debugger_command_input in) {
	auto& work = in.folder.commanded->work;
	auto& cosm = work.world;

	in.clear_dead_entity(created_id);

	if (const auto handle = cosm[created_id]) {
		cosmic::undo_last_create_entity(cosm[created_id]);
	}

	created_id = {};
}
