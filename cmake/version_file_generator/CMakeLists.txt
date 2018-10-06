cmake_minimum_required(VERSION 3.2)

project(version_file_generator)
message("Building version_file_generator.")

if(UNIX)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -std=gnu++1z"
	)
elseif(MSVC)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} /Ob2 /GL /Oi /O2 /permissive- /std:c++latest"
	)
	
	set(CMAKE_EXE_LINKER_FLAGS
		"${CMAKE_EXE_LINKER_FLAGS} /LTCG"
	)
endif()

if(CLANG)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -stdlib=libc++"
	)
endif()

include_directories("../../src/")

if(UNIX)
    add_definitions(-DPLATFORM_UNIX=1)
elseif(MSVC)
	add_definitions(-DPLATFORM_WINDOWS=1)

	add_definitions(
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS
		-D_SCL_SECURE_NO_WARNINGS
		-DUNICODE
	  )
endif()

add_executable(version_file_generator 
	"version_file_generator.cpp"
	"../../src/augs/misc/time_utils.cpp"
)

if (CLANG)
	target_link_libraries(version_file_generator c++fs c++experimental)
elseif(GCC)
	target_link_libraries(version_file_generator stdc++fs)
endif()