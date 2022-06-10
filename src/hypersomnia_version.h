#pragma once
#include <string>
#include <vector>

/*
	This will always be written to the beginning of the savefiles so that we know their version for sure.
	The layout of this structure should stay the same for long time to come.
*/

namespace augs {
	template <unsigned const_count>
	class constant_size_string;
}

using game_version_identifier = augs::constant_size_string<20>;

struct hypersomnia_version {
	hypersomnia_version();
	// GEN INTROSPECTOR struct hypersomnia_version
	unsigned commit_number;
	std::string commit_message;
	std::string commit_date;
	std::string commit_hash;
	std::vector<std::string> working_tree_changes;
	// END GEN INTROSPECTOR

	std::string get_summary() const;
	std::string get_version_string() const;

	bool operator==(const hypersomnia_version& b) const {
		return commit_hash == b.commit_hash;
	}
};

bool is_more_recent(const std::string& next_version, const std::string& current_version);