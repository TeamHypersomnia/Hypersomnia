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

include_directories("../../src/")

add_executable(version_file_generator 
	"version_file_generator.cpp"
	"../../src/augs/misc/time_utils.cpp"
)

if(UNIX)
	target_link_libraries(version_file_generator stdc++fs)
endif()