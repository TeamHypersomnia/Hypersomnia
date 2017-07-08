if(WIN32)
	if(CONFIG STREQUAL "Release")
		set(GIT_ARGS rev-list --count master)
		execute_process(COMMAND ${GIT_EXE} ${GIT_ARGS} OUTPUT_VARIABLE COMMIT_NUMBER)

		# We need to strip the output commit number of the trailing newline,
		# so that the output path does not contain a newline.

		string(REGEX REPLACE "\n$" "" COMMIT_NUMBER "${COMMIT_NUMBER}")

		set(NEW_ARCHIVE_OUTPUT_PATH "${ARCHIVE_OUTPUT_DIR}/Hypersomnia-${COMMIT_NUMBER}.7z")
		set(ARCHIVER_EXECUTABLE_PATH $ENV{ProgramW6432}/7-Zip/7z.exe)

		# We need to call the python script,
		# as CMake does not offer a civilized way of calling shell,
		# and the WinRar command breaks for some reason.

		# First we configure the script to use the correct input and output paths.
		# This should substitute ARCHIVER_EXECUTABLE_PATH, NEW_ARCHIVE_OUTPUT_PATH, RESOURCES_DIR, RESOURCES_FOLDER_NAME and EXE_PATH.

		configure_file(
			${CREATE_ARCHIVE_PY_PATH}
			gen_create_archive.py
			@ONLY
		)
		
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${EXE_PATH} ${RESOURCES_DIR})
		execute_process(COMMAND ${PYTHON_EXE} gen_create_archive.py)
	endif()
endif()