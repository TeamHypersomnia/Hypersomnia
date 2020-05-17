#pragma once

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#include <unistd.h>
#include <libgen.h>

augs::path_type get_executable_path();

static auto get_bundle_directory() {
			CFBundleRef mainBundle = CFBundleGetMainBundle();
		CFURLRef exeURL = CFBundleCopyBundleURL(mainBundle);
		char path[PATH_MAX];
		if (!CFURLGetFileSystemRepresentation(exeURL, TRUE, (UInt8 *)path, PATH_MAX))
		{
			// error!
		}

		CFRelease(exeURL);

return augs::path_type(path);
}

#define NEW_HYPERSOMNIA (get_bundle_directory() / "Contents.new")
#define OLD_HYPERSOMNIA (get_bundle_directory() / "Contents.old")

#else

#define NEW_HYPERSOMNIA "NEW_HYPERSOMNIA"
#define OLD_HYPERSOMNIA "OLD_HYPERSOMNIA"

#endif
