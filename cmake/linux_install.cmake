if(LINUX AND NOT BUILD_FOR_WEB AND NOT HEADLESS)
	set(HYPERSOMNIA_APP_ID "io.hypersomnia.Hypersomnia")
	set(HYPERSOMNIA_METAINFO_TEMPLATE "${PROJECT_SOURCE_DIR}/cmake/${HYPERSOMNIA_APP_ID}.metainfo.xml.in")
	set(HYPERSOMNIA_CHANGELOG "${PROJECT_SOURCE_DIR}/hypersomnia/CHANGELOG.md")

	install(TARGETS Hypersomnia RUNTIME DESTINATION bin)

	install(
		DIRECTORY
			"${PROJECT_SOURCE_DIR}/hypersomnia/content"
			"${PROJECT_SOURCE_DIR}/hypersomnia/detail"
		DESTINATION share/hypersomnia
	)

	install(
		FILES
			"${PROJECT_SOURCE_DIR}/hypersomnia/default_config.json"
			"${HYPERSOMNIA_CHANGELOG}"
		DESTINATION share/hypersomnia
	)

	file(READ "${PROJECT_SOURCE_DIR}/cmake/Hypersomnia.desktop" HYPERSOMNIA_DESKTOP_ENTRY)
	string(REGEX REPLACE "\nIcon=[^\n]*" "\nIcon=${HYPERSOMNIA_APP_ID}" HYPERSOMNIA_DESKTOP_ENTRY "${HYPERSOMNIA_DESKTOP_ENTRY}")
	file(WRITE "${CMAKE_BINARY_DIR}/${HYPERSOMNIA_APP_ID}.desktop" "${HYPERSOMNIA_DESKTOP_ENTRY}")

	install(FILES "${CMAKE_BINARY_DIR}/${HYPERSOMNIA_APP_ID}.desktop" DESTINATION share/applications)
	install(FILES "${PROJECT_SOURCE_DIR}/cmake/${HYPERSOMNIA_APP_ID}.png" DESTINATION share/icons/hicolor/256x256/apps)

	install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.md" DESTINATION "share/licenses/${HYPERSOMNIA_APP_ID}")

	install(
		FILES "${PROJECT_SOURCE_DIR}/hypersomnia/content/README.md"
		DESTINATION "share/licenses/${HYPERSOMNIA_APP_ID}"
		RENAME CONTENT_LICENSE.md
	)

	if(EXISTS "${HYPERSOMNIA_METAINFO_TEMPLATE}")
		find_package(Python3 REQUIRED COMPONENTS Interpreter)

		set(HYPERSOMNIA_METAINFO "${CMAKE_BINARY_DIR}/${HYPERSOMNIA_APP_ID}.metainfo.xml")
		set(HYPERSOMNIA_METAINFO_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/generate_metainfo.py")

		add_custom_command(
			OUTPUT "${HYPERSOMNIA_METAINFO}"
			COMMAND "${Python3_EXECUTABLE}" "${HYPERSOMNIA_METAINFO_GENERATOR}" "${HYPERSOMNIA_METAINFO_TEMPLATE}" "${HYPERSOMNIA_CHANGELOG}" "${HYPERSOMNIA_METAINFO}"
			DEPENDS "${HYPERSOMNIA_METAINFO_GENERATOR}" "${HYPERSOMNIA_METAINFO_TEMPLATE}" "${HYPERSOMNIA_CHANGELOG}"
			COMMENT "Generating ${HYPERSOMNIA_APP_ID}.metainfo.xml with the releases from CHANGELOG.md"
		)

		add_custom_target(metainfo ALL DEPENDS "${HYPERSOMNIA_METAINFO}")

		install(FILES "${HYPERSOMNIA_METAINFO}" DESTINATION share/metainfo)
	elseif(BUILD_FOR_FLATPAK)
		message(FATAL_ERROR "A Flatpak build needs ${HYPERSOMNIA_METAINFO_TEMPLATE}.")
	else()
		message("No ${HYPERSOMNIA_METAINFO_TEMPLATE} - the metainfo will not be installed.")
	endif()
endif()
