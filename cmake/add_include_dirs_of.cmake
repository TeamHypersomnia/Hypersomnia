macro(add_include_dirs_of target_name)
	get_property(dirs TARGET ${target_name} PROPERTY INCLUDE_DIRECTORIES)
	
	foreach(dir ${dirs})
		message(STATUS "adding external include dir='${dir}'")
		include_directories(${dir})
	endforeach()
endmacro(add_include_dirs_of target_name)