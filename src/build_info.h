#pragma once
#include "hypersomnia_version.h"
#include "augs/filesystem/path.h"
#include "augs/templates/for_each_std_get.h"
#include "augs/string/string_templates.h"
#include "augs/string/get_type_name.h"

#include "augs/string/typesafe_sprintf.h"

#include "game/organization/for_each_entity_type.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/cosmos.h"
#include "augs/misc/readable_bytesize.h"

inline auto static_allocations_info() {
	return std::string(
#if STATICALLY_ALLOCATE_ENTITIES
		"STATICALLY_ALLOCATE_ENTITIES=1\n"
#else
		"STATICALLY_ALLOCATE_ENTITIES=0\n"
#endif

#if STATICALLY_ALLOCATE_ENTITY_FLAVOURS
		"STATICALLY_ALLOCATE_ENTITY_FLAVOURS=1\n"
#else
		"STATICALLY_ALLOCATE_ENTITY_FLAVOURS=0\n"
#endif
	);
}

inline auto directories_info() {
	return typesafe_sprintf(
		"LOG_FILES_DIR=%x\n"
		"GENERATED_FILES_DIR=%x\n"
		"USER_FILES_DIR=%x\n",
		LOG_FILES_DIR,
		GENERATED_FILES_DIR,
		USER_FILES_DIR
	);
}

inline auto sizeofs_info() {
	return typesafe_sprintf(
		"cosmos=%x, entity_id=%x, unversioned=%x\n",
		readable_bytesize(sizeof(cosmos)), sizeof(entity_id), sizeof(unversioned_entity_id)
	);
}

inline auto entity_types_info() {
	std::string output;

	for_each_entity_type([&](auto e){
		output += get_type_name_strip_namespace(e) + '\n';
	});

	return output;
}

inline auto complete_build_info() {
	return 
		hypersomnia_version().get_summary()
		+ "\nCompilation flags:\n"
		+ static_allocations_info()
		+ "\nOther:\n"
		+ sizeofs_info()
		+ "\nAll entity types:\n"
		+ entity_types_info()
		+ "\nDirectories:\n"
		+ directories_info()
	;
}

void dump_detailed_sizeof_information(const augs::path_type& where);

inline auto get_help_section() {
return
	std::string("Hypersomnia\nA community-centered shooter released as free software.\n") 
	+ R"(
usage: Hypersomnia [options|debugger_file_path]

Options:
    -h, --help                  Show this help and quit.
    -v, --version               Show version information along with compilation flags.
    --verify [PATH]             Verify a manually downloaded game copy at PATH.
    --verify-updater [PATH]     Verify a manually downloaded game updater at PATH. Contrary to an ordinary game copy,
                                an updater is a self-extracting archive designed to make it easy to unpack programatically.
    --signature [PATH]          Specify the SSH signature against which to verify the game or updater.
                                This flag too must be specified if either --verify or --verify-updater was set!
                                Examples:

                                --verify Hypersomnia-for-Windows.zip --signature Hypersomnia-for-Windows.zip.sig
                                --verify-updater Hypersomnia-for-Windows.exe --signature Hypersomnia-for-Windows.exe.sig

    --unit-tests-only           Perform unit tests only and quit.
    --connect [ADDRESS]         Connect to an arena server in accordance with client_start inside the config file.
                                The ADDRESS argument is optional - if specified, it will override the custom_address field from the config file.
    --server                    Host an arena server in accordance with server_start inside the config file.
                                Contrary to the --dedicated-server option, this lets you play on your own server within the same game instance.
    --dedicated-server          The same as --server, but applies some settings suitable for a dedicated server instance.
                                For example - the game will be started without a window.
                                Remember to set --no-router if you're hosting e.g. on a proper VPS without a router.
	--no-router					Disables NAT traversal for incoming connections. NAT traversal is unnecessary if you forwarded the ports,
                                plan to only play over LAN or have a proper server instance with a dedicated IP address, without a router.

If debugger_file_path is supplied and it is a directory,
the game will automatically launch the editor to try and open the project inside it, if there is one.)"
;
}

inline auto get_version_section() {
	return complete_build_info();
}
