#pragma once
#define UNICODE
#include <string> 
#include <vector>
#include <unordered_map>
#include "readdir/ReadDirectoryChanges.h"

struct lua_State;
class script {
	bool needs_reloading;

	std::string associated_string;
	bool is_associated_string_filename;

	std::vector<char> bytecode;
public:
	class reloader {
		CReadDirectoryChanges changes;
	public:
		std::unordered_map<std::wstring, script*> filename_to_script;
		std::vector<script*> get_modified_script_files();
		void add_directory(std::wstring& directory, bool subtree);
	};

	static reloader script_reloader;

	script();

	void associate_string(const std::string&);
	void associate_filename(const std::string&, 
		bool register_for_file_reloading = true);
	
	const std::string& get_associated_string() const;
	bool is_filename() const;
	const std::vector<char>& get_bytecode() const;

	bool reload_scene_when_modified;

	void set_out_of_date();

	/* refreshes bytecode, 
	does not cause recompilation if associated string was not modified
	does always cause recompilation if associated string is a filename 
	
	returns compilation error, if any.
	*/
	std::string compile(lua_State*);
	
	std::string call(lua_State*);
};