#include "augs/string/string_templates.h"
#include "application/setups/editor/commands/flavour_commands.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"
#include "application/setups/editor/commands/asset_commands.h"

#include "application/setups/editor/property_editor/widgets/pathed_asset_widget.h"

std::string create_flavour_command::describe() const {
	return built_description;
}

void create_flavour_command::redo(const editor_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);
			using flavour_type = entity_flavour<E>;

			const auto entity_type_label = str_ops(format_field_name(get_type_name<E>())).replace_all(" ", "").subject;

			auto& work = *in.folder.work;
			auto& cosm = work.world;
			auto& defs = work.viewables;

			auto& flavours = cosm.get_common_significant({}).get_flavours<E>();
			auto& new_object = base::redo(flavours);
			
			if constexpr(flavour_type::template has<invariants::sprite>()) {
				const auto provider = asset_sane_default_provider { defs };
				new_object.set(provider.construct<invariants::sprite>());
			}

			auto make_new_flavour_name = [&](const auto i) {
				return entity_type_label + "-" + std::to_string(i);
			};

			for (int i = 1; ; ++i) {
				const auto tried_name = make_new_flavour_name(i);
				bool is_free = true;

				flavours.for_each(
					[&](const auto& id, const auto& flavour) {
						if (flavour.get_name() == tried_name) {
							is_free = false;
						}
					}
				);

				if (is_free) {
					new_object.template get<invariants::name>().name = tried_name;
					built_description = typesafe_sprintf("Created flavour: %x", tried_name);
					break;
				}
			}
		}
	);
}

void create_flavour_command::undo(const editor_command_input in) {
	type_id.dispatch(
		[&](auto e) {
			using E = decltype(e);

			auto& work = *in.folder.work;
			auto& cosm = work.world;

			auto& flavours = cosm.get_common_significant({}).get_flavours<E>();

			base::undo(flavours);
		}
	);
}
