#pragma once
#include <string> 
#include <vector>

struct lua_State;
class script {
	bool needs_reloading;

	std::string associated_string;
	bool is_associated_string_filename;

	std::vector<char> bytecode;
public:
	script();

	void associate_string(const std::string&);
	void associate_filename(const std::string&);
	
	const std::string& get_associated_string() const;
	bool is_filename() const;
	const std::vector<char>& get_bytecode() const;

	/* refreshes bytecode, 
	does not cause recompilation if associated string was not modified
	does always cause recompilation if associated string is a filename 
	
	returns compilation error, if any.
	*/
	std::string compile(lua_State*);
	
	std::string call(lua_State*);
};