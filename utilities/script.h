#pragma once
#include <string> 
#include <vector>
#include <unordered_map>

namespace augs {
	struct lua_state_wrapper;

	class script {
		bool needs_recompilation;

		std::string associated_string;
		bool is_associated_string_filename;

		std::vector<char> bytecode;
		void report_errors(std::string& errors);
	public:
		lua_State* lua_state;

		script(lua_state_wrapper& owner);

		void associate_string(const std::string&);
		void associate_filename(const std::string&);

		const std::string& get_associated_string() const;
		bool is_filename() const;
		const std::vector<char>& get_bytecode() const;

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
