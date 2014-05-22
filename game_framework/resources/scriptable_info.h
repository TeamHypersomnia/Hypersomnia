#pragma once
#define UNICODE
#include <string> 
#include <vector>
#include <unordered_map>
#include "readdir/ReadDirectoryChanges.h"

namespace resources {
	struct lua_state_wrapper;

	class script {
		bool needs_recompilation;

		std::string associated_string;
		bool is_associated_string_filename;

		std::vector<char> bytecode;
		void report_errors(std::string& errors);
	public:
		class reloader {
			CReadDirectoryChanges changes;
		public:
			reloader();
			std::ostream* report_errors;

			struct script_entry {
				script* script_ptr;
				std::vector<script*> reload_dependants;
				script_entry(script* = nullptr);
			};

			std::unordered_map<std::wstring, script_entry> filename_to_script;
			std::vector<script*> get_script_files_to_reload();
			void add_directory(const std::wstring& directory, bool subtree);
		};

		static reloader script_reloader;
		lua_State* lua_state;

		static void dofile(lua_state_wrapper& lua_state, const std::string& filename);

		script(lua_state_wrapper& owner);

		void associate_string(const std::string&);
		void associate_filename(const std::string&);
		void associate_filename(const std::string&, bool register_for_file_reloading);

		void add_reload_dependant(script*);

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
		std::string compile();

		std::string call();
	};
}
