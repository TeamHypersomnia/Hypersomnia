option(BUILD_FOR_FLATPAK "Build for the Flatpak sandbox: content read from the install prefix, data in XDG_CONFIG_HOME, no self-updater, no Steam." OFF)

if(BUILD_FOR_FLATPAK)
	message("Building for Flatpak.")

	set(PREFER_LIBCXX OFF)
	set(LINK_STEAM_INTEGRATION OFF)
	set(DEVELOP_STEAM_INTEGRATION OFF)
	set(SET_CWD_TO "${CMAKE_INSTALL_PREFIX}/share/hypersomnia")

	add_definitions(-DFLATPAK_BUILD=1)
endif()
