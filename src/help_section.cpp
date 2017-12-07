#include "hypersomnia_version.h"

std::string help_contents = 
	std::string("Hypersomnia\nA community-centered shooter released as free software.\n") 
	+ hypersomnia_version().get_summary()
	+ R"(
usage: Hypersomnia [flag|file_path]

Flags:
    -h, --help                  Show this help and quit
    --unit-tests-only           Perform unit tests only and quit

    If file_path is supplied and its extension is either lua or wp,
    the file will be automatically opened in the editor.
)";
